#include <cstdlib>
#include <list>
#include <string>
#include <vector>
#include <iostream>

typedef std::string String;

//#define Array std::vector -- use c++11 template-using
template <typename T>
using Array = std::vector<T>;

//#define Pair std::pair
template <typename T1, typename T2>
using Pair = std::pair<T1, T2>;

typedef enum MetaType {
	MT_Type, MT_Variant, MT_Function
} MetaType;

typedef enum MetaClass {
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

struct Declaration {
    String file;
    int line;

    int meta_type; // 0 type 1 var 2 function
    int meta_class; // 0 void 1 primary type 2 enum 3 pointers 4 references 5 class / struct (type defines)

    // type for class first public base & var & functioin return & pointers or typedef => int/float/double
    Declaration *base_type;
    String name;

    Declaration(const String& name, MetaType mt, MetaClass mc) : name(name), meta_type(mt), meta_class(mc) {
    }

    const String& getName() const {
        return name;
    }
};

class Statement;
class Expression;
class Class;
class Value;

struct Variant : public Declaration {
    Variant *refer; // if is a pointer or a refer
    //Class *ologyClass; For an object Variant, use base_type as a Class.

    int tag;
    Map<String, int> *tags; // class or struct 

    Variant(const String& name) : Declaration(name, MT_Variant, MC_Void) { // Void means not yet set.
    }

    const Variant* getRefer() const {
        return refer;
    }

    void init(Statement* stmt) {
    }
};

struct Member : public Variant {
    Class *enclosure_type; // class or enum name
    bool isClassMember;
};

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

    Function(const String& name) : Declaration(name, MT_Function, MC_Void) { // Void means not yet set.
    }
};

class Class : public Declaration {
public:
    Array<Member> members; // for enum & class
    Array<Function> memberFunctions; // for enum & class
    Array<Declaration*> parents; // for class / struct include base
};

class Expression {
public:
//  Array<Expression*> sub_exprs;
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
    Call(Function &func) : func(func) { }

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

    Function sink("sink");
    Function source("source");
    Function foo("foo");
    Function main("main");
    // in main function statements:
    // A *a = new A();
    Variant a("a"); // Variant aa : A <-- a
    // B &b = a->g;
    Variant b("b"); // aa.g <-- b
    // foo()        // MARK aa.g.f tained
    // sink(b.f)        // SINK for aa.g.f

    // in void foo(A *z) function statements:
    // B *x = &z->g;
    // char *w = source();  // TO MARK zz.g.f tained
    // x->f = w;

    return 0;
}


