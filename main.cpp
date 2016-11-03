/**
 * Copyright zhudiqi 2016
 * GNU GPL v3 License.
 */

#include <cstdlib>
#include <list>
#include <string>
#include <vector>
#include <map>
#include <iostream>

typedef std::string String;

//#define Array std::vector -- use c++11 template-using
template <typename T>
using Array = std::vector<T>;

//#define Pair std::pair
template <typename T1, typename T2>
using Pair = std::pair<T1, T2>;

template <typename T1, typename T2>
using Map = std::map<T1, T2>;

typedef enum MetaType {
    MT_Type, MT_Variant, MT_Function
} MetaType;

typedef enum MetaClass {
    MC_Void, MC_PrimaryType, MC_Enum, MC_Pointer, MC_Reference, MC_Class
} MetaClass;

typedef enum PrimaryType {
	PT_Void, PT_Int8, PT_UInt8, PT_Int16, PT_UInt16, PT_Int32, PT_UInt32, PT_Int64, PT_UInt64,
	PT_Char = PT_Int8, PT_UChar = PT_UInt8,
	PT_Short = PT_Int16, PT_Ushort = PT_UInt16,
	PT_Int = PT_Int64, PT_UInt = PT_UInt64,
	PT_LongInt = PT_Int64, PT_ULongInt = PT_UInt64,
	PT_LongLongInt = PT_Int64, PT_ULongLongInt = PT_UInt64,
	PT_Float, PT_Double, PT_LongDouble = PT_Double
} PrimaryType;

struct Declaration {
    String file;
    int line;

    MetaType meta_type; // 0 type 1 var 2 function
    MetaClass meta_class; // 0 void 1 primary type 2 enum 3 pointers 4 references 5 class / struct (type defines)

    // type for class first public base & var & functioin return & pointers or typedef => int/float/double
    Declaration *base_type;
    String name;

    Declaration(const String& name, Declaration *base_type, MetaType mt, MetaClass mc)
        : name(name), base_type(base_type), meta_type(mt), meta_class(mc) {
    }

    const String& getName() const {
        return name;
    }
};

class Statement;
class Expression;
class Class;
class Member;
class Function;
//class Value;

class Class : public Declaration {
public:
    Map<const String, Member> member_templates; // template members (Variant) for enum & class
    Array<Function*> methods; // for enum & class
    Array<Declaration*> parents; // for class / struct include base

    Class(const String& name) : Declaration(name, nullptr, MT_Type, MC_Class) {
    }

    bool setupClassMember(const String& memb_name, Class& memb_Clazz, bool isClassMember = false);
    bool setupPointerMember(const String& memb_name, PrimaryType refered_data_type, uint8_t referDeep = 1, Class* clazz = nullptr, bool isClassMember = false);
};

struct Variant : public Declaration {
    Variant *refer_; // if is a pointer or a refer
    //Class *ologyClass; For an object Variant, use base_type as a Class.

    int tag_;
    //Map<const String, int> *tags; // class or struct
    Map<const String, Member> *members; // for enum & class

    Variant(const String& name) : Declaration(name, nullptr, MT_Variant, MC_Void), refer_(nullptr), tag_(0) { // Void means not yet set.
        std::cout << "Instance variant name " << name << std::endl;
    }

    Variant(const String& name, Class& clazz) : Declaration(name, &clazz, MT_Variant, MC_Class), refer_(nullptr), tag_(0) { // a real object
        std::cout << "Instance class.name = " << clazz.name << " memb_name " << name << std::endl;
        members = new Map<const String, Member>(clazz.member_templates); // ?? *members = clazz.member_templates;
    }                                                                                                                                                                          

	// TODO Distinguish Primary or Primary/Class Pointer
    Variant(const String& name, PrimaryType refered_data_type, uint8_t referDeep = 0, Class* clazz = nullptr)
		: Declaration(name, nullptr, MT_Variant, MC_PrimaryType), refer_(nullptr), tag_(0) { // Void + Deep = 0 means not yet set.
        std::cout << "Instance variant name " << name << std::endl;
    }

    Variant(const Variant& rhs) : Declaration(rhs.name, rhs.base_type, rhs.meta_type, rhs.meta_class) {
        refer_ = rhs.refer_;
        tag_ = rhs.tag_;
        if (meta_class == MC_Class) {
            std::cout << "Class var Copy!! " << rhs.name  << std::endl;
        members = new Map<const String, Member>(static_cast<Class*>(rhs.base_type)->member_templates);
        } else {
            std::cout << "var Copy!! " << rhs.name << std::endl;
            members = nullptr;
        }
    }

private:

    //friend class Member;

    //Variant() : Declaration("", nullptr, MT_Variant, MC_Void) {
    //}

    //Variant(Variant* some) : Declaration("", nullptr, MT_Variant, MC_Void) { // Void
    //}

public:

    static Variant null_Variant;

    const Variant* getRefer() const {
        return refer_;
    }

    bool assign(Variant& var) {
        // TODO not assign meta_class, ONLY Judge it the same:
		meta_class = var.meta_class;
        if (var.meta_class == MC_Pointer || var.meta_class == MC_Reference) {
            refer_ = var.refer_;
        } else {
            refer_ = nullptr;
        }
        tag_ = var.tag_;
        return true;
    }

    bool point(Variant& dest) {
        meta_class = MC_Pointer;
        if (dest.meta_class != MC_Void) {
            refer_ = &dest;
        } else {
            refer_ = nullptr;
        }
        return true;
    }

    bool refer(Variant& dest) {
        if (dest.meta_class != MC_Void) {
			std::cout << "REFER_TAG for " << dest.name << " is " << dest.getRefTag() << std::endl;
            meta_class = MC_Reference;
            refer_ = &dest;
            return true;
        } else {
			std::cout << "ERROR_REFER source name is " << name << std::endl;
		}
        return false;
    }

    void tag(int t) {
        if ((meta_class == MC_Pointer || meta_class == MC_Reference) && refer_) {
            refer_->tag_ = t;
        } else {
            tag_ = t;
        }
    }

    int getRefTag() {
        if ((meta_class == MC_Pointer || meta_class == MC_Reference) && refer_) {
            return refer_->tag_;
        } else {
            return tag_;
        }
    }

    Variant& member(const String& name); // for class

    void init(Statement* stmt) {
    }
};

Variant Variant::null_Variant("");

struct Member : public Variant {
    Class* enclosure_type; // class or enum name
    bool isClassMember;

    Member(const String& name, Class& clazz, Class& enclosure_type, bool isClassMember = false)
        : Variant(name, clazz), enclosure_type(&enclosure_type), isClassMember(isClassMember) {
    }

    Member(const String& name, PrimaryType refered_data_type, uint8_t referDeep, Class* clazz, Class& enclosure_type, bool isClassMember = false)
        : Variant(name, refered_data_type, referDeep, clazz), enclosure_type(&enclosure_type), isClassMember(isClassMember) {
    }

//    Member(const Member& rhs) = default;
#if 0
     {
        refer_ = rhs.refer_;
        tag_ = rhs.tag_;
        /*
        if (meta_class == MC_Class) {
            //members = new Map<const String, Member>(rhs.enclosure_type->member_templates); // *members = clazz.member_templates;
            members = rhs.members;
        } else {
            members = nullptr;
        }
        */
    }
#endif

//    Member& operator=(const Member&) = default;
};

Variant& Variant::member(const String& name) { // for class or pointer/reference of class
    if (refer_ && refer_->meta_class == MC_Class && refer_->members) {
        std::cout << "RCLASS_MEMB:" << refer_->name << ">" << name << ", and this name is:" << this->name << std::endl;
        Map<const String, Member>::iterator it = refer_->members->find(name);
        if (it != refer_->members->end()) {
			std::cout << "[TAG=" << it->second.getRefTag() << "[MC=" << it->second.meta_class << std::endl;
            return it->second;
        }
    } else if (meta_class == MC_Class && members) {
        std::cout << "CLASS_MEM:" << this->name << ">" << name << std::endl;
        Map<const String, Member>::iterator it = members->find(name);
        if (it != members->end()) {
            return it->second;
        }
    }
    std::cout << "NO_CLASS_MEM:" << "?THIS=" << this->name << ",?REF@" << refer_  << "=" << refer_->name << ", >" << name << std::endl;
    return null_Variant;
}

class Statement {
public:
    String file;
    int line;

    Expression* expr;
};

class Function : public Declaration {
public:
    Class *clazz;
    Array<Variant> parameters; // for binding with Call(arguments)
    Array<Statement> stmts;

    Function(const String& name) : Declaration(name, nullptr, MT_Function, MC_Void) { // Void means not yet set.
    }

    Variant& invoke(Variant& a) {
        std::cout << "input tag = " << a.getRefTag() << " name=" << a.name << ", refer_to=" << (a.refer_? a.refer_->name : "") << std::endl;
        if (a.getRefTag()) {
            std::cout << "polution source" << std::endl;
        }
        return a;
    }
};

bool Class::setupClassMember(const String& memb_name, Class& memb_Clazz, bool isClassMember) {
    std::cout << "Register " << memb_name << " to " << " class " << name << std::endl;
    member_templates.insert(std::make_pair(memb_name, Member(memb_name, memb_Clazz, *this)));
    return false;
}

bool Class::setupPointerMember(const String& memb_name, PrimaryType refered_data_type, uint8_t referDeep, Class* clazz,  bool isClassMember) {
    std::cout << "Register " << memb_name << " to " << " class " << name << std::endl;
    member_templates.insert(std::make_pair(memb_name, Member(memb_name, refered_data_type, referDeep, clazz, *this)));
    return false;
}

class Expression {
public:
//	Array<Expression*> sub_exprs;
};

//class Reference : Expression {
//  Declaration *single_decl;
//  String ref_name; // "" for anonymous refer
//};

class Value : public Expression {
public:
    Variant *variant;
    String value;
};

class ConstValue : public Expression {
public:
    String value;
};

class Call : public Expression {
public:
    Call(Function &func) : func(func) {
    }

    Function &func;
    Array<Expression*> arguments;
};

class Operator : public Expression {
public:
    Array<Expression*> arguments;
};

int main()
{
    const char *source_rules[] = { "source" };
    const char *sink_rules[] = { "sink" };

    // model to be analyzed
    Array<Declaration*> global_decls;
    Array<Declaration*> current_scope_decls;
    Array<Statement*> current_scope_stmts;

    Array<Declaration*> sources_db; // declare
    Array<Declaration*> sink_db; // use

    // Try generate a sourcecode model
    // Now only demo test.cpp structue

    FILE *f = fopen("test.cpp", "r");
    if (!f) {
        perror("fopen");
    }
    fseek(f, 0, SEEK_END);
    size_t bytes = ftell(f);
    std::cout << "Size=" << bytes << std::endl;
    rewind(f);
    size_t size = bytes + 1;
    char *content = (char*)malloc(size);
    size_t r = fread(content, 1, bytes, f);
    content[size-1] = '\0';
    std::cout << "Read=" << r << std::endl;
    char c, *p = content;

    String last_token, func_token, class_token;
    char token[512] = { 0 };
    char *t = token;
    size_t num = 0, num2 = 0;
    while ((c = *p++) != '\0') {
        if (isalnum(c)) {
            *t++ = c;
            continue;
        }
        // else {
        *t++ = '\0';
        if (token[0] != '\0') {
            last_token = token;
            //std::cout << "TOKEN:" << last_token << std::endl;
        } else {
        }
        //std::cout << "SEP:" << c << std::endl;
        switch (c) {
        case '(':
            func_token = last_token;
            num = 1;
            num2 = 0;
            while ((c = *p++) != '\0') {
                if (c == '(') {
                    ++num;
                } else if (c == ')') {
                    --num;
                } else if (!num) {
                    if (c == '{') {
                        ++num2;
                    } else if (c == '}') {
                        --num2;
                        if (!num2) {
                            std::cout << "FUNCTION:" << func_token << std::endl;
                            break;
                        } else {
                            std::cout << "STATMENT_C@" << num2 << std::endl;
                        }
                    } else if (c == ';' && num2 == 1) {
                        std::cout << "STATMENT_S@" << num2 << std::endl;
                    }
                }
            }
            break;
        case '{':
            class_token = last_token;
            //std::cout << "MAYCLASS:" << class_token << std::endl;
            num = 1;
            while ((c = *p++) != '\0') {
                if (c == '{')
                    ++num;
                else if (c == '}') {
                    --num;
                    if (!num) {
                        std::cout << "CLASS:" << class_token << std::endl;
                        break;
                    } else {
                        std::cout << "CLASS_FUNCTION_OR_STATMENT_C@" << num << std::endl;
                    }
                } else if (c == ';' && num == 1) {
                    std::cout << "CLASS_DECARATION@" << num << std::endl;
                }
            }
            break;
        default:
            break;
        }
        t = token;
        //}
        if (c == '\0')
            break;
    }
    free(content);

    // Playground to check the source code model, manully setup one to check

    std::cout << "S0" << std::endl;
    Function sink("sink");
    Function source("source");
    Function foo("foo");
    Function main("main");

    // in main function statements:
    // A *a = new A();
    std::cout << "S0.1" << std::endl;
    Class A("A"), B("B");
    std::cout << "S0.2" << std::endl;
    A.setupClassMember("g", B);
    std::cout << "S0.3" << std::endl;
    B.setupPointerMember("f", PT_Char); // char* f;
    std::cout << "S.4" << std::endl;
    Variant aa("aa", A);
    std::cout << "S.5" << std::endl;
    Variant a("a");     // Variant aa : A <-- a
    std::cout << "S.6" << std::endl;
    a.point(aa);
    std::cout << "S.7" << std::endl;
    // B &b = a->g;
    Variant b("b");             // aa.g <-- b
    std::cout <<"S1" << std::endl;
    b.refer(aa.member("g"));    // b.assign(a->g);  // should refer to aa.g

    //sink.invoke(b.member("f"));

    // foo(a)                   // MARK aa.g.f tained
    foo.invoke(a);
    // in void foo(A *z) function statements:
    Variant z("z");
    z.point(aa);                // z.assign(a);		// should point to aa
    // B *x = &z->g;
    Variant x("x");
    std::cout <<"S2" << std::endl;
    x.point(z.member("g"));
    // char *w = source();      // TO MARK zz.g.f tained
    Variant w("w", PT_Char, 1); 
    w.assign(source.invoke(w)); // TODO w not correct
    w.tag(1);					// mimic polution source
    std::cout << "w.RefTag =" << w.getRefTag() << std::endl;
    // x->f = w;
    std::cout <<"S3" << std::endl;
    x.member("f").assign(w); 
    std::cout <<"S4" << std::endl;
    std::cout << "f.RefTag =" << x.member("f").getRefTag() << ">>>   DONE" << std::endl;

    // sink(b.f)        // SINK for aa.g.f
    std::cout <<"S5" << std::endl;
	Variant rf("rf"); rf.refer(b.member("f")); b.member("f").tag(3);
	std::cout << "now tag is " << b.member("f").getRefTag() << std::endl;
    sink.invoke(b.member("f"));
    sink.invoke(rf);
	aa.member("g").tag(2);
    sink.invoke(b);

    return 0;
}
