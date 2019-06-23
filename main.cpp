#include <cstdlib>
#include <list>
#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <log/log.h>
#include "logger.hpp"

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
    MT_Type, MT_Variable, MT_Function
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
        : meta_type(mt), meta_class(mc), base_type(base_type), name(name) {
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
    Map<const String, Member> member_templates; // template members (Variable) for enum & class
    Array<Function*> methods; // for enum & class
    Array<Declaration*> parents; // for class / struct include base

    Class(const String& name) : Declaration(name, nullptr, MT_Type, MC_Class) {
    }

    bool setupClassMember(const String& memb_name, Class& memb_Clazz, bool isClassMember = false);
    bool setupPointerMember(const String& memb_name, PrimaryType refered_data_type, uint8_t referDeep = 1, Class* clazz = nullptr, bool isClassMember = false);
};

class Variable : public Declaration {
public:
    Variable *refer_; // if is a pointer or a refer
    //Class *ologyClass; For an object Variable, use base_type as a Class.

    int tag_;
    //Map<const String, int> *tags; // class or struct
    Map<const String, Member> *members; // for enum & class

    Variable(const String& name) : Declaration(name, nullptr, MT_Variable, MC_Void), refer_(nullptr), tag_(0) { // Void means not yet set.
        std::cout << "Instance variant name " << name << std::endl;
    }

    Variable(const String& name, Class& clazz) : Declaration(name, &clazz, MT_Variable, MC_Class), refer_(nullptr), tag_(0) { // a real object
        std::cout << "Instance class.name = " << clazz.name << " memb_name " << name << std::endl;
        members = new Map<const String, Member>(clazz.member_templates); // ?? *members = clazz.member_templates;
    }                                                                                                                                                                          

	// TODO Distinguish Primary or Primary/Class Pointer
    Variable(const String& name, PrimaryType refered_data_type, uint8_t referDeep = 0, Class* clazz = nullptr)
		: Declaration(name, nullptr, MT_Variable, MC_PrimaryType), refer_(nullptr), tag_(0) { // Void + Deep = 0 means not yet set.
        std::cout << "Instance variant name " << name << std::endl;
    }

    Variable(const Variable& rhs) : Declaration(rhs.name, rhs.base_type, rhs.meta_type, rhs.meta_class) {
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

    //Variable() : Declaration("", nullptr, MT_Variable, MC_Void) {
    //}

    //Variable(Variable* some) : Declaration("", nullptr, MT_Variable, MC_Void) { // Void
    //}

public:

    static Variable null_Variable;

    const Variable* getRefer() const {
        return refer_;
    }

    bool assign(Variable& var) {
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

    bool point(Variable& dest) {
        meta_class = MC_Pointer;
        if (dest.meta_class != MC_Void) {
            refer_ = &dest;
        } else {
            refer_ = nullptr;
        }
        return true;
    }

    bool refer(Variable& dest) {
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

    Variable& member(const String& name); // for class

    void init(Statement* stmt) {
    }
};

Variable Variable::null_Variable("");

class Member : public Variable {
public:
    Class* enclosure_type; // class or enum name
    bool isClassMember;

    Member(const String& name, Class& clazz, Class& enclosure_type, bool isClassMember = false)
        : Variable(name, clazz), enclosure_type(&enclosure_type), isClassMember(isClassMember) {
    }

    Member(const String& name, PrimaryType refered_data_type, uint8_t referDeep, Class* clazz, Class& enclosure_type, bool isClassMember = false)
        : Variable(name, refered_data_type, referDeep, clazz), enclosure_type(&enclosure_type), isClassMember(isClassMember) {
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

Variable& Variable::member(const String& name) { // for class or pointer/reference of class
    if (refer_ && refer_->meta_class == MC_Class && refer_->members) {
        LOGD << "RCLASS_MEMB:" << refer_->name << ">" << name << ", and this name is:" << this->name << std::endl;
        Map<const String, Member>::iterator it = refer_->members->find(name);
        if (it != refer_->members->end()) {
			LOGD << "[TAG=" << it->second.getRefTag() << "[MC=" << it->second.meta_class << std::endl;
            return it->second;
        }
    } else if (meta_class == MC_Class && members) {
        LOGD << "CLASS_MEM:" << this->name << ">" << name << std::endl;
        Map<const String, Member>::iterator it = members->find(name);
        if (it != members->end()) {
            return it->second;
        }
    }
    LOGD << "NO_CLASS_MEM:" << "?THIS=" << this->name << ",?REF@" << refer_  << "=" << refer_->name << ", >" << name << std::endl;
    return null_Variable;
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
    Array<Variable> parameters; // for binding with Call(arguments)
    Array<Statement> stmts;

    Function(const String& name) : Declaration(name, nullptr, MT_Function, MC_Void) { // Void means not yet set.
    }

    Variable& invoke(Variable& a) {
        LOGI << "input tag = " << a.getRefTag() << " name=" << a.name << ", refer_to=" << (a.refer_? a.refer_->name : "") << std::endl;
        if (a.getRefTag()) {
            LOGE << "polution source" << std::endl;
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
    Variable *variant;
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

#define PRE_MATCH(c, p) (((c) == '/' && (*(p) == '/' || *(p) == '*')) || (c) == '"' || (c) == '\'' || (c) == '#')

bool skipIfMatch(char &c, char *&p)
{
	bool escape = false;
	char* q = p;
	switch (c) {
		case '/':
			if (*p == '/') {
				LOGI << "COMMNET-S";
				while ((c = *p++) != '\0' && c != '\n') {
				}
				LOGI << "END-COMMNET-S" << std::endl;
			} else if (*p == '*') {
				LOGI << "COMMNET-M";
				while ((c = *p++) != '\0' && !(c == '*' && *p == '/')) {
				}
				if (c == '*')
					c = *p++;
				LOGI << "END-COMMNET-M" << std::endl;
			}
			break;
		case '\'':
			LOGI << "LITERAL-CHAR";
			while ((c = *p++) != '\0' && c != '\'') {
			}
			LOGI << "END-LITERAL-CHAR" << std::endl;
			break;
		case '"':
			// skip all string
			LOGI << "LITERAL-STRING";
			while ((c = *p++) != '\0') {
				LOGE << "[" << c;
				if (escape) {
					escape = false;
				} else if (c == '\\') {
					escape = true;
				} else if (c == '"') {
					LOGI << "END-LITERAL-STRING" << std::endl;
					break;
				} else if (c == '\n') {
					LOGE << "ERROR-END-LITERAL-STRING" << std::endl;
					break;
				}
			}
			escape = false; // always release escape.
			LOGI << "OUT-END-LITERAL-STRING" << std::endl;
			break;
		case '#':
			// skip all macro defines
			// NOTE: escape should also ocurs in String if the macro have *OR* in the End of line.
			LOGI << "MACRO";
			while ((c = *p++) != '\0') {
				if (c == '\n') {
					if (escape) {
						escape = false;
					} else
						break;
				} else if (c == '\\') {
					escape = true;
				} else if (escape && !(c == ' ' || c == '\t' || c == '\r')) {
					escape = false;
				}
			}
			escape = false; // always release escape.
			LOGI << "END-MACRO" << std::endl;
			break;
		default:
			break;
	}
	if (c == '\0')
		--p;
	return p > q;
}

char* parseStatement(char* p)
{
	return nullptr;
}

char* parseFunction(char* p)
{
	char c;
	int num2 = 0;
	while ((c = *p++) != '\0') {
		if (PRE_MATCH(c, p) && skipIfMatch(c, p))
			continue;

		switch (c) {
			case ' ': // skip spaces
			case '\t':
			case '\n':
			case '\r':
				//break;
				continue;
			case '{': // first expect
				LOGE << "Get expect " << c << ",nextc=" << *p << std::endl;
				++num2;
				break;
			case '}':
				--num2;
				if (!num2) { // END
					//break;
					return p;
				} else {
					std::cout << "STATMENT_C@" << num2 << std::endl;
				}
				break;
			case ';':
				if (num2 == 1) {
					std::cout << "STATMENT_S@" << num2 << std::endl;
				}
				break;
			default:
				if (num2 == 0) {
					LOGE << "STOP at " << c << ",nextc=" << *p << std::endl;
					return nullptr;
				}
				break;
		}
	}
	return p;
}

int main(int argc, char **argv)
{
	log_init(argv[0], stdout, kLogDebug);

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
    size_t num = 0; //, num2 = 0;

    while ((c = *p++) != '\0') {
        if (isalnum(c)) {
            *t++ = c;
            continue;
        }
        // else { //*t++ = '\0';
        *t = '\0';
        t = token;

		//LOGD2 << (token[0] != '\0' ? token : "") << (isblank(c) ? ' ' : c);
        if (token[0] != '\0') {
            last_token = token;
            LOGD2 << "`" << last_token << "`";
        }// else {
        	//LOGD2 << "SEP[" << (c == '\n' ? "\\" : "") << (c == '\n' ? 'n' : c) << "]" << std::endl;
        //}
		LOGD2 << (isspace(c) ? ' ' : c);

		// if with NO hint for separating function.
		if (isspace(c))
			continue;

		if (PRE_MATCH(c, p) && skipIfMatch(c, p)) {
			continue;
		}

		switch (c) {
			case '(': // Function Parameters Begin
				LOGE << "FUNCTION-START" << std::endl;
				func_token = last_token;
				num = 1;
				//num2 = 0;
				while ((c = *p++) != '\0') {
					if (PRE_MATCH(c, p) && skipIfMatch(c, p))
						continue;

					if (c == '(') { // Begin parentensis in params list
						++num;
					} else if (c == ')') { // End parentensis in params list, OR Function Parameters End.
						--num;
						if (!num) { // Body
							LOGE << "Try Parse function " << func_token  << std::endl;
							char* q = parseFunction(p);
							if (q == nullptr) {
								LOGE << "Break for parse error! position at: TODO"  << std::endl;
								break;
							} else {
								p = q;
								std::cout << "FUNCTION:" << func_token << std::endl;
								break; // End function, continue Outter-Loop
							}
						}
					} else {
					}
				}
				break;
			case '{':
				LOGE << "CLASS-START" << std::endl;
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
							break; // End class, continue Outter-Loop
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
        //t = token;
        //}
        if (c == '\0') {
			LOGI << "<PRE-EOF>";
            break;
		}
    }
	LOGI << "<EOF>" << std::endl;
    free(content);

    // Playground to check the source code model, manully setup one to check

    LOG_D("S0.1begin\n");
    std::cout << "S000" << std::endl;
    LOG_D("S0.1end\n");
    Function sink("sink");
    Function source("source");
    Function foo("foo");
    Function main("main");

    // in main function statements:
    // A *a = new A();
    LOG_D("S0.1");
    Class A("A"), B("B");
    LOG_D("S0.2");
    A.setupClassMember("g", B);
    LOG_D("S0.3");
    B.setupPointerMember("f", PT_Char); // char* f;
    LOG_D("S.4");
    Variable aa("aa", A);
    LOG_D("S.5");
    Variable a("a");     // Variable aa : A <-- a
    LOG_D("S.6");
    a.point(aa);
    LOG_D("S.7");
    // B &b = a->g;
    Variable b("b");             // aa.g <-- b
    LOGD << "S1" << std::endl;
    b.refer(aa.member("g"));    // b.assign(a->g);  // should refer to aa.g

    //sink.invoke(b.member("f"));

    // foo(a)                   // MARK aa.g.f tained
    foo.invoke(a);
    // in void foo(A *z) function statements:
    Variable z("z");
    z.point(aa);                // z.assign(a);		// should point to aa
    // B *x = &z->g;
    Variable x("x");
    LOGD << "S2" << std::endl;
    x.point(z.member("g"));
    // char *w = source();      // TO MARK zz.g.f tained
    Variable w("w", PT_Char, 1); 
    w.assign(source.invoke(w)); // TODO w not correct
    w.tag(1);					// mimic polution source
    LOGD << "w.RefTag =" << w.getRefTag() << std::endl;
    // x->f = w;
    LOGD << "S3" << std::endl;
    x.member("f").assign(w); 
    LOGD << "S4" << std::endl;
    LOGD << "f.RefTag =" << x.member("f").getRefTag() << ">>>   DONE" << std::endl;

    // sink(b.f)        // SINK for aa.g.f
    LOGD <<"S5" << std::endl;
	Variable rf("rf"); rf.refer(b.member("f")); b.member("f").tag(3);
	LOGD << "now tag is " << b.member("f").getRefTag() << endl;
	LOGD << "Invoke1:" << endl;
    sink.invoke(b.member("f"));
	LOGD << "Invoke2:" << endl;
    sink.invoke(rf);
	aa.member("g").tag(2);
	LOGD << "Invoke3:" << endl;
    sink.invoke(b);

    return 0;
}

// AnaC
        if (args.size() == 4) {
            std::string arg1 = args[1];
            std::string bufName;
            size_t pos;
            if ((pos = arg1.find("sizeof")) == 0) {
                size_t nameLen = arg1.size()-6;
                size_t nameBeg = pos+6;
                if (arg1[pos+6] == '(') {
                    nameLen -= 2;
                    nameBeg += 1;
                }
                bufName = arg1.substr(nameBeg, nameLen);
                trim(bufName);
                if (bufName != args[0]) {
                    vuln = true;
                }
            } else {
                std::string arg0 = args[0];
                auto it = g_cbSizeMap.find(arg0);
                if (it != g_cbSizeMap.end()) {
                    if (it->second != arg1) {
                        vuln = true;
                    }
                }
            }
        }


// parseF
            if (c.typ == TK_O_PLUS && n && n->typ == TK_UNKOWN && last && last->typ == TK_UNKOWN) { // MP. bufferName + intVariable
                bool vuln = false;
                auto end = g_cbSizeMap.end();
                decltype(end) itLhs, itRhs;
                if ((itLhs = g_cbSizeMap.find(getTokenStr(*last))) != end ||
                    (itRhs = g_cbSizeMap.find(getTokenStr(*n))) != end) {
                    if (itRhs != end) {
                        //getRangeOfVar(*last, stmtNodes);
                    } else {
                        //getRangeOfVar(*n, stmtNodes);
                    }
                    if (stmtNodes) {
                        vuln = false;
                    } else {
                        vuln = true;
                    }
                }
                if (vuln) {
                    printf("\t#unsafe Out-Of-Range-Add. %s:%d:%d\n", file, c.line, c.column);
                }
            }

///////////// parseF
            // Members declarations / Variables
            if (n && n->typ == TK_O_LBRACKET && c.typ == TK_UNKOWN && last && (last->typ < TK_SEMICOLUMN || last->typ == TK_O_MUL)) {
                Token *n2 = NULL;
                if (ts+2 < te) {
                    n2 = &tokens[ts+2];
                }
                if (n2) {
                    std::string bufName = getTokenStr(c);
                    std::string bufSizeStr = getTokenStr(*n2);
                    // int bufSize = atoi(bufSizeStr.c_str());
                    // printf("\t^%d,%d ", last->typ, bufSize);
                    // printfToken(c);
                    // printf("\n");
                    g_bufferSizeMap[bufName] = bufSizeStr;
                }
            }

            // Calls
            bool compStmtTok = false;
            if (braceDepth > 0 && (c.typ == TK_IF || c.typ == TK_WHILE || c.typ == TK_FOR || c.typ == TK_DO || c.typ == TK_SWITCH)) {
                compStmtTok = true;
            }
            if (braceDepth >= 0 && n && n->typ == TK_O_LPARENT && !(compStmtTok ||
                c.typ >= TK_O_ASSIGN || c.typ == TK_SINGLELINE_COMMENT ||
                c.typ == TK_MULTILINES_COMMENT)) {
                // printfToken(c);
                if (braceDepth == 0) {
                    head = &c;
                    headIndex = ts;
                } else if (braceDepth > 0) {
                    analyzeCalls(tokens, ts, te, c, file);
                }
            }

            // Function declarations
            //*
            if (head && n && braceDepth == 0) {  // && stmtDepth == -1
                if (n->typ == TK_O_LPARENT) {
                    bool vuln = false;
                    std::vector<std::pair<std::string, std::string> > params;
                    parseFuncParams(tokens, ts+2, te, params);
                    for (const auto& param : params) {
                        printf("\t###(%s :%s -- %s)\n", param.first.c_str(), param.second.c_str(), file);
                    }
                } else if (n->typ == TK_LBRACE && c.typ == TK_O_RPARENT) {
                    stmtDepth = 0;
                    printf("@");
                    printfToken(*head);
                    printf("\n");
                    func = new Function(nullptr, headIndex, getTokenStr(*head), braceDepth);
                    stmtNodes = new CmptStmt(stmtNodes, ts+1, getTokenStr(*n), braceDepth);
                    func->setBody(stmtNodes);
                    funcs.push_back(func);
                    head = nullptr;
                }
            }
            //*/

            // Statements
            CmptStmt* stmtNode;
            if (compStmtTok) {
                stmtNode = new CmptStmt(stmtNodes, ts, getTokenStr(c), braceDepth);
                /*
                if (stmtNodes != nullptr) {
                    //static_cast<Node*>(stmtNode)->setParent(a); // hide Node* setParent
                    stmtNode->setParent(stmtNodes);
                } else {
                    stmtNode->setParent(nullptr);
                }*/
                stmtNodes = stmtNode;
            } else if (stmtNodes && braceDepth == stmtNodes->getBraceDepth() && (c.typ == TK_SEMICOLUMN || (c.typ == TK_RBRACE))) { // (last && last->typ == TK_RBRACE)
                if (c.typ == TK_SEMICOLUMN) {
                    Stmt* exprStmtNode = new Stmt(stmtNodes, ST_EXPR, ts, ";", braceDepth);
                }
                stmtNode = stmtNodes;
                stmtNodes = stmtNode->getParent();
                // delete stmtNode;
            } else if (stmtNodes && c.typ == TK_SEMICOLUMN) {
                //stmtNodes->addChild(
                Stmt* exprStmtNode = new Stmt(stmtNodes, ST_EXPR, ts, ";", braceDepth);
            }

            // Statements match

            // ARR30-C. Do not form or use out-of-bounds pointers or array subscripts
            // ARR30-C case2: Dereferencing Past-the-End Pointer: while (cond) { *pointerVar++ }
            if (stmtNodes) {
                const Token* tok = stmtNodes->Tok(tokens);
                if (tok && tok->typ == TK_WHILE) { // MP. while (some-Condition)
                    if (n && n->typ == TK_O_INC && last && last->typ == TK_O_MUL) { // MS. *pointerVariable++
                        std::vector<std::string> args;
                        size_t ti = tok - &tokens[0];
                        parseCallArgs(tokens, ti+2, te, args); // J1. parse Condition
                        if (args.size() > 0 && !strstr(args[0].c_str(), getTokenStr(c).c_str())) { // J. Condition contains pointerVariable
                            printf("\t#unsafe Out-Of-Range. %s:%d:%d\n", file, c.line, c.column);
                        }
                    }
                }
            }

            // Buffer Analysis
            // ARR30-C case1: Forming Out-of-Bounds Pointer: arr + may-out-of-bounds-index
            // ARR30-C case3: Using Past-the-End Index: arr[may-out-of-bounds-index]
            if (c.typ == TK_O_PLUS && n && n->typ == TK_UNKOWN && last && last->typ == TK_UNKOWN) { // MP. bufferName + intVariable
                bool vuln = false;
                auto end = g_bufferSizeMap.end();
                decltype(end) itLhs, itRhs;
                if ((itLhs = g_bufferSizeMap.find(getTokenStr(*last))) != end ||
                    (itRhs = g_bufferSizeMap.find(getTokenStr(*n))) != end) {
                    if (itLhs != end) {
                        getRangeOfVar(*n, stmtNodes, tokens);
                    } else {
                        getRangeOfVar(*last, stmtNodes, tokens);
                    }
                    if (stmtNodes) {
                        vuln = false;
                    } else {
                        vuln = true;
                    }
                }
                if (vuln) {
                    printf("\t#unsafe Out-Of-Range-Add. %s:%d:%d\n", file, c.line, c.column);
                }
            }
////////////////

//parseCallArgs
 
//parseFuncParams


class Node {
public:
    ~Node() {}

    Node(Node* parent, size_t tokIndex, const std::string& name, int braceDepth, bool isStatement = false)
        : parent(parent), isStatement(isStatement), tokIndex(tokIndex), name(name), braceDepth(braceDepth) {
    }

    Node* getParent() {
        return this->parent;
    }

    Node* setParent(Node* parent) {
        Node* temp = parent;
        this->parent = parent;
        return temp;
    }
    
    int getBraceDepth() {
        return this->braceDepth;
    }

    const Token* Tok(const std::vector<Token>& tokens) {
        return (tokIndex < tokens.size()) ? &tokens[tokIndex] : nullptr;
    }

private:
    friend class Stmt;
    friend class TypeNode;

    class Node* parent;
    bool isStatement : 1;
    std::string name;
    size_t tokIndex;
    int braceDepth;
};

typedef enum StatmentType {
    ST_EXPR = 0, ST_DECLARE, ST_COMPOSITE, ST_IF, ST_ELSEIF, ST_ELSE, ST_SWITCH, ST_CASE, ST_DEFAULT,
    ST_FOR, ST_FOR_INIT, ST_FOR_COND, ST_FOR_STEP, ST_WHILE, ST_DO, // ST_IF_INIT, ST_SWITCH_INIT, ST_WHILE_INIT,
    ST_RETURN, ST_BREAK, ST_CONTINUE, ST_LABEL, ST_GOTO, 
} StatmentType;

typedef enum Operator {
    OP_CALL, OP_COMMA
} Operator;

class Stmt : public Node {
public:
    Stmt(class CmptStmt* parent, size_t tokIndex, const std::string& name, int braceDepth);
    class CmptStmt* getParent();
    //virtual bool setParent(class CmptStmt* parent);

private:
    StatmentType type;
};

class Expr : Stmt { // Leaf of The Statement Tree
    Operator op;
};

class BinaryExpr : Expr {
    Expr* lhs;
    Expr* rhs;
};

class UnaryExpr : Expr {
    Expr* oprand;
};

typedef union {
    char charVal;
    short shortVal;
    int intVal;
    long longVal;
    long long longLongVal;
    unsigned char ucharVal;
    unsigned short ushortVal;
    unsigned int uintVal;
    unsigned long ulongVal;
    unsigned long long ulongLongVal;
    float floatVal;
    double doubleVal;
    long double longDoubleVal;
    void* pointerVal;
    uintptr_t uintptrVal;
} Any;

typedef enum {
    VT_CHAR, VT_INT8, VT_SHORT, VT_INT16, VT_INT32, VT_INT, VT_LONG, VT_LONGLONG, VT_INT64,
    VT_UCHAR, VT_UINT8, VT_USHORT, VT_UINT16, VT_UINT32, VT_UINT, VT_ULONG, VT_ULONGLONG, VT_UINT64,
    VT_UINTPTR, VT_FLOAT, VT_DOUBLE, VT_LONGDOUBLE, VT_POINTER, VT_ARRAY, VT_STRUCT_CLASS, VT_VIRTUAL_CLASS
} ValueType;

class ConstOrVariable : Expr {
    std::string name;
    ValueType valType;
    Any value; // or initial value for Variable
};

class Const : ConstOrVariable {
};

// class FunctionName : Const {
// };

class Variable : ConstOrVariable {
};

class PrimaryVariable : Variable {
};

typedef enum MetaType {
    MT_STRUCT, MT_MEMBER, MT_FUNCTION, MT_METHOD, 
} MetaType;

class TypeNode : Node {
public:
    //TypeNode(class TypeNode* parent, size_t tokIndex, const std::string& name, int braceDepth);
    TypeNode(class TypeNode* parent, size_t tokIndex, const std::string& name, int braceDepth)
        : Node(parent, tokIndex, name, braceDepth, true) {
        // if (parent) {
        //     parent->addChild(this);
        // }
    }

    class TypeNode* getParent();

private:
    MetaType type;
};

class Function : TypeNode {
public:
    Function(class TypeNode* parent, size_t tokIndex, const std::string& name, int braceDepth)
        : TypeNode(parent, tokIndex, name, braceDepth), body(nullptr) {
    }

    void setBody(CmptStmt* body) {
        this->body = body;
    }

private:
    // std::string name;
    class CmptStmt* body;
};

class Method : Function {
};

class Member : TypeNode {
};

class Struct : TypeNode {
    std::vector<Member> staticMembers;
    std::vector<Function*> staticFunctions;
    std::vector<Member> members;
    std::vector<Method*> methods;
};

class StructVariable : Variable {
    Struct* structTypeInfo;
};

class Buffer {
    Variable* refToFirstElem;
    size_t length;
};

class ArrayVariable : Variable {
    Buffer original;
    ValueType elemType;
};

class PointerVariable : Variable {
    Buffer *original;
    size_t currentIndex;
};

// class Pointer : PointerOrArray {
// };

class CallExpr : Expr {
    Expr* lhs;
    std::vector<Expr*> args;
};

class AssignStmt : BinaryExpr {
};

class CommaExpr : Expr {
    std::vector<Expr*> children;
};

class DeclareStmt : Stmt {
};

class CmptStmt : public Stmt {
public:
    CmptStmt(CmptStmt* parent, size_t tokIndex, const std::string& name, int braceDepth)
        : Stmt(parent, tokIndex, name, braceDepth) {
    }

    int addChild(Stmt* stmt) {
        if (!stmt)
            return -1;
        children.push_back(stmt);
        stmt->setParent(this);
        return children.size()-1;
    }

    size_t getChildCount() {
        return children.size();
    }

private:
    friend class Stmt;
    std::vector<Stmt*> children;
};

Stmt::Stmt(CmptStmt* parent, size_t tokIndex, const std::string& name, int braceDepth)
    : Node(parent, tokIndex, name, braceDepth, true) {
    if (parent) {
        parent->addChild(this);
    }
}

CmptStmt* Stmt::getParent() {
    return static_cast<CmptStmt*>(this->parent);
}

/*
bool Stmt::setParent(CmptStmt* parent) {
    if (!parent) {
        return false;
    }
    Node* temp = Node::setParent(parent);
    // parent->children.push_back(this);
    if (!temp) {
        delete temp;
    }
    return true;
}
*/

class IfStmt : CmptStmt {
    CmptStmt* branchStmt; // nullptr | else | else if
};

class ElseIfStmt : CmptStmt {
    CmptStmt* branchStmt; // nullptr | else | else if
};

class ElseStmt : CmptStmt {
};

class ForStmt : CmptStmt {
    Stmt* init; // AssignStmt | CommaExpr | DeclareStmt
    Expr* cond; // Expr
    Stmt* step;
};

class WhileStmt : CmptStmt {
    Stmt* init; // AssignStmt | CommaExpr | DeclareStmt
    Expr* cond; // Expr
};

void printAST(CmptStmt* body) {
    for (auto& child : body->children) {
        printf("\t%s\n", child->getName().c_str());
        if (child->isComposite()) {
            printAST(static_cast<CmptStmt*>(child));
        }
    }
}

std::unordered_map<std::string, std::string> g_bufferSizeMap;

//analyzeCalls

std::vector<Function*> funcs;
Function* func = nullptr;

bool getRangeOfVar(const Token& var, const Node* stmtNodes, const std::vector<Token>& tokens) {

}

///////////////////
//parseFile


            if (stmtNodes && n && n->typ == TK_O_INC && last && last->typ == TK_O_MUL) { // MP1. *pointerVariable++
                // const Token* tok = stmtNodes->tokenIn(tokens);
                // if (tok && tok->typ == TK_WHILE) { // MP2. while (some-Condition)
                std::vector<CmptStmt*> parents = stmtNodes->getParentWith(ST_WHILE);
                if (parents.size() > 0) {
                    //if (n && n->typ == TK_O_INC && last && last->typ == TK_O_MUL) { // MS. *pointerVariable++
                    std::vector<std::string> args;
                    // const Token* tok =parents[0]->tokenIn(tokens);
                    // size_t ti = tok - &tokens[0];
                    size_t ti = parents[0]->getTokIndex();
                    parseCallArgs(tokens, ti+2, te, args); // J1. parse Condition
                    if (args.size() > 0 && !strstr(args[0].c_str(), getTokenStr(c).c_str())) { // J. Condition contains pointerVariable
                        printf("\t#unsafe Out-Of-Range. %s:%d:%d\n", file, c.line, c.column);
                    }
                    //}
                }
            }



class CmptStmt : public Stmt {
public:
    CmptStmt(CmptStmt* parent, size_t tokIndex, TokenType tokType, const std::string& name, int braceDepth)
        : Stmt(parent, ST_COMPOSITE, tokIndex, tokType, name, braceDepth) {
    }

protected:
    CmptStmt(CmptStmt* parent, StatmentType stmtType, size_t tokIndex, TokenType tokType, const std::string& name, int braceDepth)
        : Stmt(parent, stmtType, tokIndex, tokType, name, braceDepth) {
    }

public:
    static CmptStmt* createDerivedCmptStmt(CmptStmt* parent, size_t tokIndex, TokenType tokType, const std::string& name, int braceDepth);

    int addChild(Stmt* stmt) {
        if (!stmt)
            return -1;
        children.push_back(stmt);
        stmt->setParent(this);
        return children.size()-1;
    }

    size_t getChildCount() {
        return children.size();
    }

    std::vector<CmptStmt*> getParentWith(StatmentType stmtType) { // TokenType tokenType, const std::vector<Token>& tokens
        std::vector<CmptStmt*> result;
        // return (tokIndex < tokens.size()) ? &tokens[tokIndex] : nullptr;
        // const Token* tok = stmtNodes->tokenIn(tokens);
        // if (tok && tok->typ == TK_WHILE)
        CmptStmt* parent = this;
        while (parent) {
            if (parent->stmtType == stmtType) {
                result.push_back(parent);
            }
            parent = parent->getParent();
        }
        return result;
    }

private:
    friend class Stmt;
    friend void printAST(CmptStmt* body);
    friend void destroyAST(CmptStmt* body);
    std::vector<Stmt*> children;
};

Stmt::Stmt(CmptStmt* parent, StatmentType stmtType, size_t tokIndex, TokenType tokType, const std::string& name, int braceDepth)
    : Node(parent, tokIndex, tokType, name, braceDepth, true), stmtType(stmtType) {
    if (parent) {
        parent->addChild(this);
    }
}

CmptStmt* Stmt::getParent() {
    return static_cast<CmptStmt*>(this->parent);
}

/*
bool Stmt::setParent(CmptStmt* parent) {
    if (!parent) {
        return false;
    }
    Node* temp = Node::setParent(parent);
    // parent->children.push_back(this);
    if (!temp) {
        delete temp;
    }
    return true;
}
*/

class IfStmt : public CmptStmt {
public:
    IfStmt(CmptStmt* parent, size_t tokIndex, TokenType tokType, const std::string& name, int braceDepth)
        : CmptStmt(parent, ST_IF, tokIndex, tokType, name, braceDepth) {
    }

private:
    CmptStmt* branchStmt; // nullptr | else | else if
};

class ElseIfStmt : public CmptStmt {
    CmptStmt* branchStmt; // nullptr | else | else if
};

class ElseStmt : public CmptStmt {
};

class ForStmt : public CmptStmt {
public:
    ForStmt(CmptStmt* parent, size_t tokIndex, TokenType tokType, const std::string& name, int braceDepth)
        : CmptStmt(parent, ST_FOR, tokIndex, tokType, name, braceDepth) {
    }

private:
    Stmt* init; // AssignStmt | CommaExpr | DeclareStmt
    Expr* cond; // Expr
    Stmt* step;
};

class WhileStmt : public CmptStmt {
public:
    WhileStmt(CmptStmt* parent, size_t tokIndex, TokenType tokType, const std::string& name, int braceDepth)
        : CmptStmt(parent, ST_WHILE, tokIndex, tokType, name, braceDepth) {
    }

private:
    Stmt* init; // AssignStmt | CommaExpr | DeclareStmt
    Expr* cond; // Expr
};

CmptStmt* CmptStmt::createDerivedCmptStmt(CmptStmt* parent, size_t tokIndex, TokenType tokType, const std::string& name, int braceDepth) {
    CmptStmt* result = nullptr;
    switch (tokType) {
        case TK_IF:
            result = new IfStmt(parent, tokIndex, tokType, name, braceDepth);
            break;
        case TK_FOR:
            result = new ForStmt(parent, tokIndex, tokType, name, braceDepth);
            break;
        case TK_WHILE:
            result = new WhileStmt(parent, tokIndex, tokType, name, braceDepth);
            break;
        default:
            result = new CmptStmt(parent, tokIndex, tokType, name, braceDepth);
            break;
    }
    return result;
}


void destroyTypeStack(TypeNode* typeNodes) {
     while (typeNodes) {
        TypeNode* typeNode = typeNodes;
        typeNodes = typeNodes->getParent();
        delete typeNode;
    }
}

void destroyStmtStack(CmptStmt* stmtNodes) {
     while (stmtNodes) {
        Stmt* stmtNode = stmtNodes;
        stmtNodes = stmtNodes->getParent();
        delete stmtNode;
    }
}

std::unordered_map<std::string, std::string> g_bufferSizeMap;
