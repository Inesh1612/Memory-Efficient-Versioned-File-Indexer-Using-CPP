#include<bits/stdc++.h>
using namespace std;

template <typename T> T to_lowercase(T str){
    T lower_str=str;
    for(auto& c: lower_str){
        c=tolower(c);
    }
    return lower_str;
}

class buffer_size_exception: public runtime_error{
public:
    buffer_size_exception(const string& message): runtime_error(message){}
};

class version_index{
private:
    string version_name;
    unordered_map<string,int> word_frequency;

public:
    version_index(const string& vers_name): version_name(vers_name){}

    void add_word(const string& word){
        word_frequency[word]++;
    }

    void add_word(const string& word, int count){
        word_frequency[word]+=count;
    }

    int get_frequency(const string& word)const{
        auto it=word_frequency.find(word);
        if(it!=word_frequency.end()){
            return it->second;
        }else{
            return 0;
        }
    }

    const unordered_map<string,int>& get_frequencies()const{
        return word_frequency;
    }

    unordered_map<string,int>&get_frequencies_ref(){
        return word_frequency;
    }

    string get_version_name()const{
        return version_name;
    }
};

class tokenizer{
public:
    vector<string>tokenize(const char* data, size_t len, string& left)const{
        vector<string>tokens;
        string cur=std::move(left);
        left.clear();
        for(size_t i=0;i<len;i++){
            unsigned char c=data[i];
            if(isalnum(c)){
                cur+=tolower(c);
            }else if(!cur.empty()){
                tokens.push_back(std::move(cur));
                cur.clear();
            }
        }
        left=std::move(cur);
        return tokens;
    }

    void tokenize(const char*data, size_t len, string& left, unordered_map<string,int>& freq)const{
        string cur=std::move(left);
        left.clear();
        for(size_t i=0;i<len;i++){
            unsigned char c=data[i];
            if(isalnum(c)){
                cur+=tolower(c);
            }else if(!cur.empty()){
                freq[cur]++;
                cur.clear();
            }
        }
        left=std::move(cur);
    }
};

class buffered_file_reader{
private:
    size_t buffer_size_bytes;

public:
    buffered_file_reader(size_t kb_size){
        if(kb_size<256||kb_size>1024){
            throw buffer_size_exception("Buffer size is invalid.");
        }
        buffer_size_bytes=kb_size*1024;
    }

    void read_file(const string& file_path, version_index& index){
        ifstream file(file_path, ios::binary);
        if(!file){
            throw runtime_error("Failed to open file: "+file_path);
        }
        vector<char> buffer(buffer_size_bytes);
        tokenizer token;
        string left_word="";
        while(file){
            file.read(buffer.data(),buffer_size_bytes);
            size_t bytes_read=file.gcount();
            if(bytes_read>0){
                token.tokenize(buffer.data(),bytes_read,left_word,index.get_frequencies_ref());
            }
        }
        if(!left_word.empty()){
            index.add_word(left_word);
        }
    }
};

class query{
public:
    virtual void execute()const=0;
    virtual ~query()=default;
};

class word_count_query:public query{
private:
    const version_index& index;
    string target_word;

public:
    word_count_query(const version_index& index, const string& word):index(index),target_word(to_lowercase(word)){}
    void execute()const override{
        cout<<"Version: "<<index.get_version_name()<<endl;
        cout<<"Count: "<<index.get_frequency(target_word) << endl;
    }
};

class top_k_query: public query{
private:
    const version_index& index;
    int k;

public:
    top_k_query(const version_index& index, int top_k):index(index),k(top_k){}
    void execute()const override{
        vector<pair<string,int>>freq_vector(index.get_frequencies().begin(),index.get_frequencies().end());
        sort(freq_vector.begin(),freq_vector.end(),[](const pair<string,int>&a, const pair<string,int>& b){
            if(a.second!=b.second) return a.second>b.second;
            return a.first<b.first;
        });
        cout<<"Top-"<<k<<" words in version "<<index.get_version_name()<<":"<<endl;
        int count=0;
        for(const auto&pair:freq_vector){
            if(count>=k)break;
            cout<<pair.first<<" "<<pair.second<<endl;
            count++;
        }
    }
};

class difference_query:public query{
private:
    const version_index& index1,index2;
    string target_word;

public:
    difference_query(const version_index& index1, const version_index& index2, const string& word):index1(index1),index2(index2),target_word(to_lowercase(word)){}
    void execute()const override{
        int frequency1=index1.get_frequency(target_word);
        int frequency2=index2.get_frequency(target_word);
        int difference=frequency2-frequency1;
        cout<<"Difference ("<<index2.get_version_name()<<" - "<<index1.get_version_name()<<"): "<<difference<<endl;
    }
};

class query_processor{
public:
    void process(const query* query){
        query->execute();
    }
};

int main(int argc,char* argv[]){
    auto start_time=chrono::high_resolution_clock::now();
    
    unordered_map<string,string>args;
    for(int i=1;i<argc;i+=2){
        if(i+1<argc){
            args[argv[i]]=argv[i+1];
        }
    }
    try{
        if(args.count("--buffer")==0)throw runtime_error("Buffer size must be provided.");
        if(args.count("--query")==0)throw runtime_error("Query type must be provided.");

        int kb_buffer=stoi(args["--buffer"]);
        string query_type=args["--query"];
        
        buffered_file_reader reader(kb_buffer);
        query_processor processor;
        if(query_type=="word"||query_type=="top"){
            version_index index(args["--version"]);
            reader.read_file(args["--file"],index);
            if(query_type=="word"){
                word_count_query q(index,args["--word"]);
                processor.process(&q);
            }else if(query_type=="top"){
                top_k_query q(index,stoi(args["--top"]));
                processor.process(&q);
            }
        } 
        else if(query_type=="diff"){
            version_index index1(args["--version1"]);
            version_index index2(args["--version2"]);
            reader.read_file(args["--file1"],index1);
            reader.read_file(args["--file2"],index2);
            difference_query q(index1,index2,args["--word"]);
            processor.process(&q);
        } 
        else{
            throw runtime_error("Invalid query type provided. Use word or top or diff");
        }
        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> execution_time=end_time-start_time;
        cout<<"Buffer size (KB): "<<kb_buffer<<endl;
        cout<<"Execution time (s): "<<execution_time.count()<<endl;
    }catch(const exception& e){
        cerr<<"Error: "<<e.what()<<endl;
        return 1;
    }
    return 0;
}