#include <stdio.h>
#include <string>
#include <iostream>
#include <list>
#include <fstream>
#include <sstream>
#include "json.hpp"


using namespace nlohmann;

enum Token_Type {
    delimiter, statement, text, number, method, method_1, method_n, ident, btn, 
    qstn, gift, domain, param_int, param_str, param_1, param_2, answer_1, answer_2,  
    response_1, response_2, color, size, bgcolor, param_RGB, empty
};

std::string token_types[25] = {"delimiter", "statement", "text", "number", "method", "method_1", "method_n", "parentheses", "btn", 
    "qstn", "gift", "domain", "param_int", "param_str", "param_1", "param_2", "answer_1", "answer_2",  
    "response_1", "response_2", "color", "size", "bgcolor", "param_RGB", "empty"};

struct Token
{  
    Token_Type type = empty; 
    std::string symbols;
};

struct Tree
{
    Token token;
    int counter = 0;
    Tree *children[5] = {NULL};
};

void print_tree(Tree *t){
    std::cout<<"<<token:"<<token_types[t->token.type]<<" | symbols:\""<<t->token.symbols<<"\""<<std::endl;
    int i=0;
    while(t->children[i] != NULL){
        std::cout<<"i="<<i<<std::endl;
        print_tree(t->children[i]);
        i++;
    }
    std::cout<<"no more children\n";
}

std::string error_msg = "";

bool str_is_int(std::string str){
    for(int digit_idx = 0; digit_idx < str.length(); digit_idx++){
        if(str[digit_idx] < 48 || str[digit_idx] > 57)
            return false;
    }

    return true;
}

std::list<Token> check_points(std::list<Token> tokens){
    std::list<Token> new_tokens;
    std::list<Token>::reverse_iterator tk;
    std::string tmp = "";
    bool need_point = false, need_text_and_points = false, clear_tmp = false;
    for (tk = tokens.rbegin(); tk != tokens.rend(); tk++) {
        if(tk->type == method || tk->type == method_1 || tk->type == method_n){
            need_point = true;
        }

        if(need_text_and_points && (tk->type == statement || (tk->type == ident && tk->symbols.compare(".") == 0))){
            tmp = tk->symbols + tmp;
            continue;
        }else{
            need_text_and_points = false;
            clear_tmp = true;
        }

        if(need_point && tk->type == ident && tk->symbols.compare(".") == 0){
            need_point = false;
            need_text_and_points = true;
        }

        if(tmp.length()!=0){
            Token pf_token;
            pf_token.symbols = tmp;
            pf_token.type = statement;
            new_tokens.push_front(pf_token);
        }

        Token pf_token;
        pf_token.symbols = tk->symbols;
        pf_token.type = tk->type;
        new_tokens.push_front(pf_token);

        if(clear_tmp) {
            clear_tmp = false;
            tmp = "";
        }
            
    }

    if(tmp.length()!=0){
        Token pf_token;
        pf_token.symbols = tmp;
        pf_token.type = statement;
        new_tokens.push_front(pf_token);
    }

    return new_tokens;
}

std::list<Token> Lexer(std::string message){

    // TODO: split the code into tokens
    std::list<Token> tokens;
    std::cout<<"Splitting string \""<<message<< "\" into tokens:\n";
    
    // handy variables for lexer
    std::string tmp = "";
    bool token_pushed = false, need_function = false, need_parameter = false, non_parameter_function = false, need_opened_paran = false, need_square_parentheses = false,
		need_str_param = false, need_curly = false, need_rgb_param = false;
    int is_question = 0;
    
    // lexer
    for (int i = 0; i < message.length(); i++){
        if(message[i] == '+'){
            // create token object and push to token list
            Token temp_token;
            
            if(tmp.compare("")!=0){
                temp_token.symbols = tmp;
                temp_token.type = text;
                tokens.push_back(temp_token);    
            }
            
            temp_token.symbols = "+";
            temp_token.type = delimiter;
            tokens.push_back(temp_token);
            
            token_pushed = true;
        }
        if(message[i] == '.'){            
            // create token object and push to token list
            Token new_token;
            if(tmp.length() > 0){
                new_token.symbols = tmp;
                new_token.type = statement;
                tokens.push_back(new_token);
            }
            
            // push the "." identifier
            new_token.symbols = ".";
            new_token.type = ident;
            tokens.push_back(new_token);
            
            // after "." was found, search for a funtion name
            need_function = true;            
            token_pushed = true;
        }
        if(message[i] == '(' || message[i] == ')'){
            if(need_opened_paran){
                if(message[i] != '('){
                    error_msg = "opened parenthesis missing";
                    return tokens;
                }
                else if(message[i] == '('){
                    Token open_p_token;
                    open_p_token.symbols = "(";
                    open_p_token.type = ident;
                    tokens.push_back(open_p_token);
                }

                need_opened_paran = false;
                continue;
            }

            // treat parantheses as text
            if(!need_parameter && !non_parameter_function){
                token_pushed = false;
            }
            // treat parentheses as identifiers
            else{
                // on a function with parameters, tokenize one parameter stored in "tmp"
                if (!non_parameter_function){
                    if(tmp.length() == 0 && message[i] == ')'){
                        error_msg = "parameter is missing";
                        return tokens;
                    }
                    
                    if (!need_str_param && !need_rgb_param && !str_is_int(tmp)){
                        error_msg = "parameter is not an integer";
                        return tokens;
                    }

                    // TODO:
                    // check if parameter is RGB color
                    if(need_rgb_param){
                        need_rgb_param = false;
                    }

                    // add parameter to tokens list
                    Token param_token;
                    param_token.symbols = tmp;
                    if(need_str_param || is_question == 1){
                    	param_token.type = param_str;
					}
					else if(need_rgb_param){
                        param_token.type = param_RGB;
                    }
                    else{
						param_token.type = param_int;
					}
                    tokens.push_back(param_token);
                }
                
                // open/ close parentheses
                // on a function with/ without parameters
                Token open_p_token;
                open_p_token.symbols = message[i];
                open_p_token.type = ident;
                tokens.push_back(open_p_token);

                token_pushed = true;
                need_parameter = false;

                if (message[i] == ')')
                    non_parameter_function = false;
                
                if(is_question == 1){
                    is_question++;
                    need_square_parentheses = true;
                    continue;
                }
                    
            }
        }

        if(message[i] == '[' || message[i] == ']'){
            if(need_square_parentheses == true){
                if(message[i] != '['){
                    error_msg = "opened square bracket missing";
                    return tokens;
                }
                else if(message[i] == '['){
                    Token open_p_token;
                    open_p_token.symbols = "[";
                    open_p_token.type = ident;
                    tokens.push_back(open_p_token);
                    
                    need_square_parentheses = false;
                    token_pushed = true;
                }
            }
            if (message[i] == ']'){
                if (need_square_parentheses){
                    error_msg = "closed square parenthesis missing";
                    return tokens;
                }
                Token param_token;
                param_token.symbols = tmp;
                //std::cout<<"|||"<<is_question<<std::endl;
                if(is_question == 2)
                    param_token.type = answer_1;
                else if(is_question == 3)
                    param_token.type = answer_2;
                else if(is_question == 4)
                    param_token.type = response_1;
                else 
                    param_token.type = param_str;
                tokens.push_back(param_token);

                Token open_p_token;
                open_p_token.symbols = "]";
                open_p_token.type = ident;
                tokens.push_back(open_p_token);
                
                need_str_param = true;
                token_pushed = true;
                
                if(is_question == 2){
                    is_question++;
                    need_square_parentheses = true;
                    continue;
                }
                if(is_question == 3){
                    is_question++;
                    need_curly = true;
                    continue;
                }
            }
        }

        if(message[i] == '{' || message[i] == '}'){
            if(need_curly == true){
                if(message[i] != '{'){
                    error_msg = "opened curly braket missing";
                    return tokens;
                }
                else if(message[i] == '{'){
                    Token open_p_token;
                    open_p_token.symbols = "{";
                    open_p_token.type = ident;
                    tokens.push_back(open_p_token);
                    
                    need_curly = false;
                    need_square_parentheses = true;
                    token_pushed = true;
                }
                      
            }
            if (message[i] == '}'){
                if(need_curly){
                    error_msg = "closed curly braket missing";
                    return tokens;
                }
                Token param_token;
                param_token.symbols = tmp;
                //std::cout<<"|||"<<is_question<<std::endl;
                if(is_question == 4)
                    param_token.type = response_2;
                else 
                    param_token.type = param_str;
                tokens.push_back(param_token);

                Token open_p_token;
                open_p_token.symbols = "}";
                open_p_token.type = ident;
                tokens.push_back(open_p_token);
                
                token_pushed = true;
                
            }
        }

        
        // reset token or add curent char to token
        if(token_pushed){
            token_pushed = false;
            tmp = "";
        }else{
            // build token by adding next character
            tmp += message[i];
        }
        
        // search for the function
        if(need_function){
            if(tmp.compare("upper") == 0 || tmp.compare("lower") == 0){
                // create token object and push to token list
                Token temp_token;
                temp_token.symbols = tmp;
                temp_token.type = method;
                tokens.push_back(temp_token);

                need_function = false;
                tmp = "";   

                need_opened_paran = true;
                non_parameter_function = true;
            }
            if(tmp.compare("repeat") == 0 || tmp.compare("rightcut") == 0 || tmp.compare("leftcut") == 0){
                Token temp_token;
                temp_token.symbols = tmp;
                temp_token.type = method_1;
                tokens.push_back(temp_token);
            
                need_function = false;
                tmp = "";
                
                need_opened_paran = true;
                need_parameter = true;
            }
            if(tmp.compare("replace") == 0 ){
                Token temp_token;
                temp_token.symbols = tmp;
                temp_token.type = method_n;
                tokens.push_back(temp_token);

                need_function = false;
                tmp = "";

                need_opened_paran = true;
                need_parameter = true;
                need_square_parentheses = true;
            }
        }

        if(tmp.compare("$button") == 0){
            Token temp_token;
            temp_token.symbols = tmp;
            temp_token.type = btn;
            tokens.push_back(temp_token);

            tmp = "";   

            need_opened_paran = true;
            need_parameter = true;
            need_square_parentheses = true;
        }

        if(tmp.compare("$gift") == 0){
            Token temp_token;
            temp_token.symbols = tmp;
            temp_token.type = gift;
            tokens.push_back(temp_token);

            tmp = "";   

            need_opened_paran = true;
            need_parameter = true;
            need_str_param = true;
        }

        if(tmp.compare("$question") == 0){
            Token temp_token;
            temp_token.symbols = tmp;
            temp_token.type = qstn;
            tokens.push_back(temp_token);

            tmp = "";   

            need_opened_paran = true;
            need_parameter = true;
            need_str_param = true;
            is_question++;
        }

        if(tmp.compare("@color") == 0){
            Token temp_token;
            temp_token.symbols = "color";
            temp_token.type = color;
            tokens.push_back(temp_token);

            tmp = "";   

            need_opened_paran = true;
            need_parameter = true;
            need_rgb_param = true;
        }

        if(tmp.compare("@size") == 0){
            Token temp_token;
            temp_token.symbols = "size";
            temp_token.type = size;
            tokens.push_back(temp_token);

            tmp = "";   

            need_opened_paran = true;
            need_parameter = true;
        }

        if(tmp.compare("@bgcolor") == 0){
            Token temp_token;
            temp_token.symbols = "bgcolor";
            temp_token.type = bgcolor;
            tokens.push_back(temp_token);

            tmp = "";   

            need_opened_paran = true;
            need_parameter = true;
            need_rgb_param = true;
        }
    } 
    
    // add the left chars as text
    if(tmp.compare("")!=0){    
        Token temp_token;
        temp_token.symbols = tmp;
        temp_token.type = text;
        tokens.push_back(temp_token);
    }

    // TODO: handle errors when these are true
    // std::cout<<need_function<<std::endl;
    // std::cout<<need_parameter<<std::endl;
    // std::cout<<need_opened_paran<<std::endl;

    if(non_parameter_function){
        error_msg = "closed parenthesis missing";
        return tokens;
    }

    // check if points are simple text
    tokens = check_points(tokens);

    return tokens;
}

bool different_structure(Token_Type type){
    Token_Type structures[3] {btn, statement};
    int length = sizeof(structures)/sizeof(structures[0]);
    bool found = false;
    for(int i=0; i<length; i++){
        if(structures[i] == type)
            found = true;
    }
    return found;
}

std::string build_json_string(Tree *t){
    if(t->token.type == text){
        std::cout<<"Found text\n";
        return t->token.symbols;
    }

    int i=0;
    std::string str = "";
    std::cout<<std::endl;

    while(t->children[i] != NULL){
        //std::cout<<"NOW "<<t->token.symbols<<"("<<t->token.type<<"): |"<<str<<"|"<<std::endl;

        if(t->token.type == delimiter){
            //std::cout<<"Found delimiter\n";
            str += build_json_string(t->children[i]);
        }

        if(t->token.type == statement){
            if(t->token.symbols.compare("upper") == 0){
                //std::cout<<"Found upper\n";
                std::string u_c = t->children[i]->token.symbols;
                std::transform(u_c.begin(), u_c.end(), u_c.begin(), std::ptr_fun<int, int>(std::toupper));
                str += u_c;
            }

            if(t->token.symbols.compare("lower") == 0){
                //std::cout<<"Found lower\n";
                std::string u_c = t->children[i]->token.symbols;
                std::transform(u_c.begin(), u_c.end(), u_c.begin(), std::ptr_fun<int, int>(std::tolower));
                str += u_c;
            }

            if(t->token.symbols.compare("repeat") == 0){
                //std::cout<<"Found repeat\n";
                if(i==0){
                    //std::cout<<"getting the string\n";
                    str = t->children[i]->token.symbols;
                }
                if(i==1){
                    //std::cout<<"repeating\n";
                    int repetitions = stoi(t->children[i]->token.symbols);
                    std::string new_str = "";
                    for(int j=0; j<repetitions; j++){
                        new_str += str;
                    }
                    str = new_str;
                }
            }
            
            if(t->token.symbols.compare("replace") == 0){
                //std::cout<<"Found repeat\n";
                if(i==0){
                    //std::cout<<"getting the string\n";
                    str = t->children[i]->token.symbols;
                }
                if(i==1){
                    int index = 0;
                    while (true) {
                        index = str.find(t->children[i]->token.symbols, index);
                        if (index == std::string::npos) 
                            break;
                        //std::cout<<"found something:"<<index<<"\n";
                        str = str.substr(0, index) + t->children[i]->children[0]->token.symbols + str.substr(index+t->children[i]->token.symbols.length());
                        //std::cout<<"result:"<<str<<"\n";
                        index += t->children[i]->token.symbols.length();
                    }
                }
            }
            if(t->token.symbols.compare("leftcut") == 0){
                if(i==0){
                    //std::cout<<"getting the string\n";
                    str = t->children[i]->token.symbols;
                }
                if(i==1){
                    //std::cout<<"Found leftcut:"<<t->children[i]->token.symbols<<"\n";
                    int to_cut = stoi(t->children[i]->token.symbols);
                    str = str.substr(to_cut, str.length()-to_cut);
                }
            }
            if(t->token.symbols.compare("rightcut") == 0){
                if(i==0){
                    //std::cout<<"getting the string\n";
                    str = t->children[i]->token.symbols;
                }
                if(i==1){
                    //std::cout<<"Found rightcut:"<<t->children[i]->token.symbols<<"\n";
                    int to_cut = stoi(t->children[i]->token.symbols);
                    str = str.substr(0, str.length()-to_cut);
                }
            }
        }

        if(t->token.type == btn){
            std::string param1, param2;
            param1 = t->children[i]->token.symbols;
            param2 = t->children[i]->children[0]->token.symbols;
            str = "[\"" + param1 + "\", \"" + param2 + "\"]";
        }

        if(t->token.type == qstn){
            //std::cout<<"Found question\n";
            if(i==0){
                //std::cout<<"getting the string\n";
                str = t->children[i]->token.symbols;
            }
            if(i==1){
                //std::cout<<"getting the answers\n";
                str = "{\"text\": \"" + str + "\", ";

                std::string answ1, answ2;
                answ1 = t->children[i]->token.symbols;
                answ2 = t->children[i]->children[0]->token.symbols;

                str += "\"answers\": [\"" + answ1 + "\", \"" + answ2 + "\"], ";
            }
            if(i==2){
                //std::cout<<"getting the responses\n";

                std::string resp1, resp2;
                resp1 = t->children[i]->token.symbols;
                resp2 = t->children[i]->children[0]->token.symbols;

                str += "\"responses\": [\"" + resp1 + "\", \"" + resp2 + "\"] ";
                str += "}";
            }
        }

        i++;
    }

    if(t->token.type == delimiter)
        str = "\"" + str + "\"";

    return str;
}

json tree_to_json(Tree *t)
{
    std::string json_string = build_json_string(t);
    if(t->token.type == btn)
        json_string = "{ \"button\": " + json_string + " }";
    else if(t->token.type == qstn)
        json_string = "{ \"question\": " + json_string + " }";
    else
        //if (t->token.type == statement)
        json_string = "{ \"message\": \"" + json_string + "\" }";

    std::cout<<"R: "<<json_string<<std::endl;
    json result = json::parse(json_string);
    return result;
}

json style_to_json(std::list<Token> style_tokens)
{
    std::string json_string = "{\"styles\": {";

    // parsing styles

    Tree *root;
    Tree first_node;
    root = &first_node;
    std::list<Token>::iterator tk;
    for (tk = style_tokens.begin(); tk != style_tokens.end(); tk++) {
        if(tk->type == color || tk->type == bgcolor || tk->type == size ){
            Tree *style_object = new Tree();
            style_object -> token.type = tk -> type;
            style_object -> token.symbols = tk->symbols;
            tk++;
            tk++;
            Tree *object_param = new Tree();
            object_param -> token.type = tk -> type;
            object_param -> token.symbols = tk->symbols;
            tk++;
            style_object->children[style_object->counter] = object_param;
            style_object->counter++;

            
            root->children[root->counter] = style_object;
            root->counter++;
            
        }
    }

    //print_tree(root);
    int i=0;
    if(root->children[i] != NULL){
        json_string += "\"" + root->children[i]->token.symbols + "\": \"" + root->children[i]->children[0]->token.symbols + "\"";
        i++;
    }
    while(root->children[i] != NULL){
        json_string += ", \"" + root->children[i]->token.symbols + "\": \"" + root->children[i]->children[0]->token.symbols + "\"";
        i++;
    }

    json_string += "} }";

    //std::cout<<"R: "<<json_string<<std::endl;
    json result = json::parse(json_string);
    return result;
}

std::string analyze (std::string raw)
{
    //----LEXER------------------------------------------------------
    // message to tokens
    std::list<Token> tokens = Lexer(raw);

    // print token list
    std::cout<<" Tokens created:\n";
    std::list<Token>::iterator it;
    for (it = tokens.begin(); it != tokens.end(); it++) {
        std::cout<<"type:"<<token_types[it->type]<<" | symbols:\""<<it->symbols<<"\""<<std::endl;
    }

    // print lexer errors
    if (error_msg.length() > 0){
        std::cout<<"In Lexer:\nERROR:"<<error_msg<<std::endl;
        return "{\"error\":\""+error_msg+"\"}";
    }
    std::cout<<std::endl;
        

    //----PARSER----------------------------------------------------------
    std::cout<<"\n Tokens parsed:\n";
    Tree *root;
    Tree first_node;
    root = &first_node;
    std::list<Token>::iterator tk;
    bool delimiter_is_next = false;
    for (tk = tokens.begin(); tk != tokens.end(); tk++) {
        std::cout<<"type:"<<token_types[tk->type]<<" | symbols:\""<<tk->symbols<<"\""<<std::endl;
        
        if(tk->type == btn){
            Tree *btn_object = new Tree();
            btn_object -> token.type = tk -> type;
            btn_object -> token.symbols = tk->symbols;
            tk++;
            if(tk->symbols.compare("(") != 0) std::cout<<"ERROR: \"(\" missing\n"; 
            tk++;
            if(tk->symbols.compare("[") != 0) std::cout<<"ERROR: \"[\" missing\n"; 
            tk++;

            std::cout<<"type:"<<token_types[tk->type]<<" | symbols:\""<<tk -> symbols<<"\""<<std::endl;
            Tree *param_1_node = new Tree();
            param_1_node -> token.type = tk -> type;
            param_1_node -> token.symbols = tk->symbols;
            btn_object->children[btn_object->counter] = param_1_node;
            btn_object->counter++;
            tk++;

            if(tk->symbols.compare("]") != 0) std::cout<<"ERROR: \"]\" missing\n"; 
            tk++;
            std::cout<<"type:"<<token_types[tk->type]<<" | symbols:\""<<tk -> symbols<<"\""<<std::endl;
            Tree *param_2_node = new Tree();
            param_2_node -> token.type = tk -> type;
            param_2_node -> token.symbols = tk->symbols;
            param_1_node->children[param_1_node->counter] = param_2_node;
            param_1_node->counter++;
            tk++;
            if(tk->symbols.compare(")") != 0) std::cout<<"ERROR: \")\" missing\n"; 


            if(root->token.type == empty || root->token.type == text){
                root = btn_object;
            }else{
                root->children[root->counter] = btn_object;
                root->counter++;
            }

        }

        if(tk->type == gift){
            Tree *gift_object = new Tree();
            gift_object -> token.type = tk -> type;
            gift_object -> token.symbols = tk->symbols;
            tk++;
            if(tk->symbols.compare("(") != 0) std::cout<<"ERROR: \"(\" missing\n"; 
            tk++;

            std::cout<<"type:"<<token_types[tk->type]<<" | symbols:\""<<tk -> symbols<<"\""<<std::endl;
            Tree *param_1 = new Tree();
            param_1 -> token.type = tk -> type;
            param_1 -> token.symbols = tk->symbols;
            gift_object->children[gift_object->counter] = param_1;
            gift_object->counter++;
            tk++;

            if(tk->symbols.compare(")") != 0) std::cout<<"ERROR: \")\" missing\n"; 


            if(root->token.type == empty || root->token.type == text){
                root = gift_object;
            }else{
                root->children[root->counter] = gift_object;
                root->counter++;
            }
        }

        if(tk->type == qstn){
            Tree *qstn_object = new Tree();
            qstn_object -> token.type = tk -> type;
            qstn_object -> token.symbols = tk->symbols;
            
            // tokenize question string
            tk++;
            if(tk->symbols.compare("(") != 0) std::cout<<"ERROR: \"(\" missing\n"; 
            
            tk++;
            std::cout<<"type:"<<token_types[tk->type]<<" | symbols:\""<<tk -> symbols<<"\""<<std::endl;
            
            Tree *qstn_str = new Tree();
            qstn_str -> token.type = tk -> type;
            qstn_str -> token.symbols = tk->symbols;
            
            qstn_object->children[qstn_object->counter] = qstn_str;
            qstn_object->counter++;

            tk++;
            if(tk->symbols.compare(")") != 0) std::cout<<"ERROR: \"(\" missing\n"; 

            // tokenize answers
            tk++;
            if(tk->symbols.compare("[") != 0) std::cout<<"ERROR: \"[\" missing\n"; 

            tk++;
            std::cout<<"type:"<<token_types[tk->type]<<" | symbols:\""<<tk -> symbols<<"\""<<std::endl;
            
            Tree *answ_1 = new Tree();
            answ_1 -> token.type = tk -> type;
            answ_1 -> token.symbols = tk->symbols;
            
            qstn_object->children[qstn_object->counter] = answ_1;
            qstn_object->counter++;

            tk++;
            if(tk->symbols.compare("]") != 0) std::cout<<"ERROR: \"]\" missing\n"; 
            tk++;
            if(tk->symbols.compare("[") != 0) std::cout<<"ERROR: \"[\" missing\n"; 

            tk++;
            std::cout<<"type:"<<token_types[tk->type]<<" | symbols:\""<<tk -> symbols<<"\""<<std::endl;
            
            Tree *answ_2 = new Tree();
            answ_2 -> token.type = tk -> type;
            answ_2 -> token.symbols = tk->symbols;
            
            answ_1->children[answ_1->counter] = answ_2;
            answ_1->counter++;

            tk++;
            if(tk->symbols.compare("]") != 0) std::cout<<"ERROR: \"]\" missing\n"; 

            // tokenize responses
            tk++;
            if(tk->symbols.compare("{") != 0) std::cout<<"ERROR: \"{\" missing\n"; 
            tk++;
            if(tk->symbols.compare("[") != 0) std::cout<<"ERROR: \"[\" missing\n"; 

            tk++;
            std::cout<<"type:"<<token_types[tk->type]<<" | symbols:\""<<tk -> symbols<<"\""<<std::endl;

            Tree *response_1 = new Tree();
            response_1 -> token.type = tk -> type;
            response_1 -> token.symbols = tk->symbols;
            
            qstn_object->children[qstn_object->counter] = response_1;
            qstn_object->counter++;

            tk++;
            if(tk->symbols.compare("]") != 0) std::cout<<"ERROR: \"]\" missing\n"; 

            tk++;
            std::cout<<"type:"<<token_types[tk->type]<<" | symbols:\""<<tk -> symbols<<"\""<<std::endl;

            Tree *response_2 = new Tree();
            response_2 -> token.type = tk -> type;
            response_2 -> token.symbols = tk->symbols;
            
            response_1->children[response_1->counter] = response_2;
            response_1->counter++;

            tk++;


            if(root->token.type == empty || root->token.type == text){
                root = qstn_object;
            }else{
                root->children[root->counter] = qstn_object;
                root->counter++;
            }
        }

        if(tk->type == statement){
            // make main node
            Tree *node = new Tree();
            node->token.type = tk->type;

            // make child node with text
            Tree *text_node = new Tree();
            // if root is text, and there was no delimiter, concatenate
            if(root->token.type == text){
                text_node->token.symbols = root->token.symbols + tk->symbols;
            }else{
                text_node->token.symbols = tk->symbols;
            }
                
            text_node->token.type = text;

            // complete main node with function name
            tk++; 
            std::string ident_chars = tk->symbols;
            tk++;
            
            // if the "." character is simple text
            Token_Type method_type = tk->type;
            if(!(tk->type == method || tk->type == method_1 || tk->type == method_n)){
                tk--; tk--;
                text_node->token.symbols = text_node->token.symbols + ident_chars;
                root = text_node;
                continue;
            }

            std::cout<<"type:"<<token_types[tk->type]<<" | symbols:\""<<tk->symbols<<"\""<<std::endl;
            node->token.symbols = tk->symbols;

            // build statement tree
            node->children[node->counter] = text_node;
            node->counter++;
            
            // search the "("
            tk++;
            if(tk->symbols.compare("(") != 0) std::cout<<"ERROR: \"(\" missing\n";            
            
            //parsing parameters
            tk++; 

            // for parsing method with one parameter
            if ((method_type == method_1 || method_type == method_n) && tk -> type == param_int){
            	Tree *param_node = new Tree();
            	param_node -> token.type = param_int;
        		param_node -> token.symbols = tk->symbols;		

				// make param_node the children of "method node"
				node->children[node->counter] = param_node;
				node->counter++;

				// show that the parameter was parsed
				std::cout<<"type:"<<token_types[param_node -> token.type]<<" | symbols:\""<<param_node -> token.symbols<<"\""<<std::endl;	

				tk++; //skip the ")"        		
			}
			
            // for parsing multi-parameter method and button
			if((method_type == method_n) && tk -> type == ident && tk -> symbols == "["){
                // get next token: first parameter
				tk++;
				
				std::cout<<"type:"<<token_types[tk->type]<<" | symbols:\""<<tk -> symbols<<"\""<<std::endl;
				
				Tree *param_1_node = new Tree();
            	param_1_node -> token.type = tk -> type;
        		param_1_node -> token.symbols = tk->symbols;	
        		
        		node->children[node->counter] = param_1_node;
            	node->counter++;
        		
                // get next token: ]
        		tk++;
                // get next token: second parameter
        		tk++;
        		
        		std::cout<<"type:"<<tk -> type<<" | symbols:\""<<tk -> symbols<<"\""<<std::endl;
        		
        		Tree *param_2_node = new Tree();
            	param_2_node -> token.type = tk -> type;
        		param_2_node -> token.symbols = tk->symbols;
        		
        		param_1_node->children[param_1_node->counter] = param_2_node;
            	param_1_node->counter++;

                // skip next token: )
            	tk++;
        		
			}
			
            // add statement tree to main tree
            if(root->token.type == empty || root->token.type == text){
                root = node;
            }else{
                root->children[root->counter] = node;
                root->counter++;
            }
        }
        
        if(tk->type == delimiter){
            // add the delimiter as parent node
            Tree *new_parent = new Tree();
            new_parent->token.symbols = tk->symbols;
            new_parent->token.type = tk->type;
            new_parent->children[0] = root;
            new_parent->counter++;
            root = new_parent; 
            
            if(delimiter_is_next){
				tk--;
				Tree *new_child = new Tree();
                new_child->token.symbols = tk->symbols;
                new_child->token.type = tk->type;
                // add child
                root->children[root->counter] = new_child;
                root->counter++;
                tk++;
			}
        }
        else if(tk->type == text){
            // root is empty -> make token as root
            if(root->token.type == empty){
                root->token.symbols = tk->symbols;
                root->token.type = tk->type;
            }
            // root is not empty -> make token as child
            else{
            	// firstly check if next one is "+", to make its child
            	tk++;
            	if(tk->type == delimiter){
            		tk--;
            		delimiter_is_next = true;
            		continue;
            	}
                tk--;
                // create child 
                Tree *new_txt_child = new Tree();
                new_txt_child->token.symbols = tk->symbols;
                new_txt_child->token.type = tk->type;
                // add child
                root->children[root->counter] = new_txt_child;
                root->counter++;
            }
        }
    }

    
    std::cout<<"\nTHE TREE\n";
    print_tree(root);
    std::cout<<"END\n\n";
      
    json return_obj = tree_to_json(root);
       
    std::string json_string = return_obj.dump();
    std::cout<<"\nRESULT: "<<json_string<<std::endl;    
    
    return json_string;
}

