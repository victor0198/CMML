#include <stdio.h>
#include <string>
#include <iostream>
#include <list>
#include <fstream>

enum Token_Type {
    delimiter, statement, text, number, method, method_1, method_n, ident, btn, 
    qstn, gft, domain, param_int, param_1, param_2, answ_1, answ_2,  
    q_param_1, q_param_2, empty
};

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
    bool token_pushed = false, need_function = false, need_parameter = false, non_parameter_function = false, need_opened_paran = false;
    
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
                    error_msg = "opened paranthese missing";
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

            // treat parantheses it as text
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

                    if (!str_is_int(tmp)){
                        error_msg = "parameter is not an integer";
                        return tokens;
                    }

                    // add parameter to tokens list
                    Token param_token;
                    param_token.symbols = tmp;
                    param_token.type = param_int;
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
                // create token object and push to token list
                Token temp_token;
                temp_token.symbols = tmp;
                temp_token.type = method_1;
                tokens.push_back(temp_token);
            
                need_function = false;
                tmp = "";
                
                need_opened_paran = true;
                need_parameter = true;
            }
        }
    } 
    std::cout<<"symbols left:"<<tmp<<std::endl;
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
        error_msg = "closed parenthese missing";
        return tokens;
    }

    // check if points are simple text
    tokens = check_points(tokens);

    return tokens;
}

void print_tree(Tree *t){
    std::cout<<"<<token:"<<" | symbols:\""<<t->token.symbols<<"\""<<std::endl;
    int i=0;
    while(t->children[i] != NULL){
        std::cout<<"i="<<i<<std::endl;
        print_tree(t->children[i]);
        i++;
    }
    std::cout<<"no more children\n";
}

int main ()
{
    // read the message from the file
    std::fstream newfile;
    std::string message;
    newfile.open("message.txt",std::ios::in); //open a file to perform read operation using file object
    if (newfile.is_open()){   //checking whether the file is open
        std::string tp;
        std::getline(newfile, tp); //read the first line from the file object and put it into string variable
        message = tp;
        newfile.close(); 
    }

    //----LEXER------------------------------------------------------
    // message to tokens
    std::list<Token> tokens = Lexer(message);

    // print token list
    std::list<Token>::iterator it;
    for (it = tokens.begin(); it != tokens.end(); it++) {
        std::cout<<"type:"<<it->type<<" | symbols:\""<<it->symbols<<"\""<<std::endl;
    }
    std::cout<<" Tokens printed\n\n";

    // print lexer errors
    if (error_msg.length() > 0){
        std::cout<<"In Lexer:\nERROR:"<<error_msg<<std::endl;
        return 1;
    }
        

    //----PARSER----------------------------------------------------------

    Tree *root;
    Tree first_node;
    root = &first_node;
    std::list<Token>::iterator tk;
    bool delimiter_is_next = false;
    for (tk = tokens.begin(); tk != tokens.end(); tk++) {
        std::cout<<"type:"<<tk->type<<" | symbols:\""<<tk->symbols<<"\""<<std::endl;

        if(tk->type == statement){
            // make main node
            Tree *node = new Tree();
            node->token.type = tk->type;

            // make child node with text
            Tree *text_node = new Tree();
            // if root is text, and there was no delimiter, concatenate
            if(root->token.type == text){
                // std::cout<<"root symbols:"<<root->token.symbols<<std::endl;
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

            std::cout<<"type:"<<tk->type<<" | symbols:"<<tk->symbols<<std::endl;
            node->token.symbols = tk->symbols;

            // build statement tree
            node->children[node->counter] = text_node;
            node->counter++;
            
            // search the "("
            tk++;
            if(tk->symbols.compare("(") != 0) std::cout<<"ERROR: \"(\" missing\n";            
            
            //parsing parameters
            tk++; 
            // check if it is a mothod with parameter
            if ((method_type == method_1 || method_type == method_n) && tk -> type == param_int){
            	Tree *param_node = new Tree();
            	param_node -> token.type = param_int;
        		param_node -> token.symbols = tk->symbols;		

				// make param_node the children of "method node"
				node->children[node->counter] = param_node;
				node->counter++;

				// show that the parameter was parsed
				std::cout<<"type:"<<param_node -> token.type<<" | symbols:"<<param_node -> token.symbols<<std::endl;	

				tk++; //skip the ")"        		
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
                // create child 
                Tree *new_child = new Tree();
                new_child->token.symbols = tk->symbols;
                new_child->token.type = tk->type;
                // add child
                root->children[root->counter] = new_child;
                root->counter++;
            }
        }
    }
    std::cout<<" Tokens parsed\n";

    std::cout<<"\nTHE TREE\n";
    print_tree(root);

    
    // END
    std::cout<<"END";
    std::getchar();
    return 0;
}

