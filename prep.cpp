#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <regex>
#include <unordered_map>
#include <functional>

using namespace std;
namespace fs = experimental::filesystem;

// temp
void read(string &, string &);
void insert_indent(string &, string &, size_t);
void trim(string &);


class Preprocessor {
	string & buf;
	string & filename;
	struct tag_replacement {
		string tag;
		size_t position;
		string replacement;
	};
	vector<tag_replacement *> tr_ptrs;
	unordered_map<string, string> definitions;

	void include(string & param, tag_replacement * tr){
		string buf;
		try {
			read(param, buf);
		} catch(runtime_error) {
			string err('\'' + tr->tag + "': unable to open input file specified");
			format_error(err, tr->position);
			cerr << err;
			throw runtime_error(err);
		}
		Preprocessor p{buf, param};
		tr->replacement = buf;
	}

	void define(string & param, tag_replacement * tr){
		tr->replacement = "";
		regex word_rx("\\S+");
		smatch mr;
		if(!regex_search(param, mr, word_rx)){
			string err('\'' + tr->tag + "': unable to extract definition identifier");
			format_error(err, tr->position);
			cerr << err;
			throw invalid_argument(err);
		}
		string id = mr.str();
		size_t str_start(mr.position() + id.size() + 1);
		string str;
		if(str_start < param.size()){
			str = param.substr(str_start);
			trim(str);
		}
		definitions[id] = str;
		/*
		word_rx.assign("\\{\\{\\s*" + id + "\\s*\\}\\}");
		while(regex_search(buf, mr, word_rx)){
			tag_replacement * t = new tag_replacement{mr.str(), unsigned int(mr.position()), true, str};
			tr_ptrs.push_back(t);
		}
		*/
	}

	typedef void (Preprocessor::*fPtr)(string &, tag_replacement *);
	unordered_map<string, fPtr> function_map{
		{"include", &Preprocessor::include},
		{"define", &Preprocessor::define}
	};

	unsigned int get_line(size_t position){
		size_t line = 1;
		for(size_t i = 0; i != position; ++i){
			if(buf[i] == '\n'){
				++line;
			}
		}
		return line;
	}

	void format_error(string & error, size_t position){
		error = (filename + "(" + to_string(get_line(position)) + "): error: " + error);
	}

	void handle_command(tag_replacement * tr){
		regex cmd_f_rx("\\S+");
		smatch mr;
		if(!regex_search(tr->tag.cbegin() + 2, tr->tag.cend() - 2, mr, cmd_f_rx)){
			string err('\'' + tr->tag + "': incorrectly formatted command");
			format_error(err, tr->position);
			cerr << err;
			throw invalid_argument(err);
		}
		string directive = mr.str();
		auto it = function_map.find(directive);
		if(it == function_map.end()){
			string err('\'' + directive + "': unrecognized command directive");
			format_error(err, tr->position);
			cerr << err;
			throw invalid_argument(err);
		}
		string params;
		size_t param_start(mr.position() + directive.size() + 2);
		size_t tag_end = tr->tag.size() - 2;
		if(param_start < tag_end){
			params = tr->tag.substr(param_start, tag_end - param_start);
			trim(params);
		}
		fPtr f = it->second;
		(this->*f)(params, tr);
	}

	void replace(tag_replacement * tr){
		int position = tr->position;
		buf.erase(position, tr->tag.size());
		insert_indent(tr->replacement, buf, position);
		buf.insert(position, tr->replacement);
	}

	public:
		Preprocessor(string & _buf, string & _filename) : buf{_buf}, filename{_filename}{
			/*
			regex cmd_rx("\\{\\{[^\\}]+\\}\\}");
			sregex_iterator it(buf.begin(), buf.end(), cmd_rx);
			sregex_iterator end;
			while(it != end){
				tag_replacement * tr = new tag_replacement{it->str(), size_t(it->position())};
				tr_ptrs.push_back(tr);
				handle_command(tr);
				++it;
			}
			*/
			int cr(0);
			for(size_t i = 0, end = buf.size() - 3; i < end; ++i){
				if(buf[i] == '{' && buf[i + 1] == '{'){
					size_t start = i;
					i += 2;
					size_t last = end + 2;
					while(i != last && (buf[i] != '}' || buf[i + 1] != '}')){
						++i;
					}
					if(i == last) {
						string err("missing closing curly braces");
						format_error(err, i);
						cerr << err;
						throw invalid_argument(err);
					}
					i += 2;
					size_t len = i - start;
					if(len){
						tag_replacement * tr = new tag_replacement{buf.substr(start, len), start};
						tr_ptrs.push_back(tr);
						handle_command(tr);
					}
				}
			}
			for(int i = tr_ptrs.size() - 1; i != -1; --i){
				replace(tr_ptrs[i]);
				delete tr_ptrs[i];
			}
			regex id_rx;
			smatch mr;
			for(auto it = definitions.begin(), end_it = definitions.end(); it != end_it; ++it){
				size_t pos = 0;
				id_rx.assign(it->first);
				while(regex_search(buf.cbegin() + pos, buf.cend(), mr, id_rx)){
					pos += mr.position();
					tag_replacement t{mr.str(), pos, it->second};
					replace(&t);
				}
			}
		}
};

void ltrim(string & str){
	for(size_t i = 0, s = str.size(); i != s; ++i){
		if(!isspace(str[i]) && str[i] != '\0'){
			if(i != 0){
				str.erase(0, i);
			}
			break;
		}
	}
}

void rtrim(string & str){
	for(long s = str.size(), i = s - 1; i != -1; --i){
		if(str[i] != '\0' && !isspace(str[i])){
			++i;
			if(i != s){
				str.erase(i);
			}
			break;
		}
	}
}

void trim(string & str){
	ltrim(str);
	rtrim(str);
}

void read(string & filename, string & buf){
	ifstream file(filename, ios::binary);
	if(!file.is_open()){
		string err("Unable to open input file");
		throw runtime_error(err);
	}
	size_t file_size = fs::file_size(filename);
	buf.reserve(file_size * 2);
	buf.resize(file_size);
	file.rdbuf()->sgetn(&buf[0], file_size);
	file.close();
	//rtrim(buf);
}

void insert_indent(string & replacement, string & buf, size_t position){
	size_t size = replacement.size();
	if(!size || !position){
		return;
	}
	size_t start_position(position);
	do {
		--position;
	} while(position && buf[position] != '\n');
	++position;
	if(position == start_position){
		return;
	}
	regex indent_rx("^\\s+");
	smatch mr;
	if(!regex_search(buf.cbegin() + position, buf.cbegin() + start_position, mr, indent_rx)){
		return;
	}
	string indent = mr.str();
	for(long i = size - 2; i > -1; --i){
		if(replacement[i] == '\n'){
			replacement.insert(i + 1, indent);
		}
	}
}

string of_name;

unordered_map<string, function<void(string)>> options_map{
	{
		"Fe",
		[](string param) -> void {
			if(!param.size()){
				string err("output filename cannot be empty");
				cerr << err;
				throw invalid_argument(err);
			}
			of_name = param;
		}
	}
};

int main(int argc, char * argv[]){
	if(argc < 2) {
		string r = "usage: prep [ option... ] filename";
		cerr << r;
		throw invalid_argument(r);
	}
	for(size_t i = 1, size = argc - 1; i != size; ++i){
		if(*argv[i] == '/'){
			string token(argv[i] + 1);
			size_t token_size = token.size();
			size_t option_end = 0;
			while(option_end < token_size && token[option_end] != ':'){
				++option_end;
			}
			string option(token, 0, option_end);
			auto it = options_map.find(option);
			if(it == options_map.end()){
				string err("error: '" + string(argv[i]) + "': unrecognized option");
				cerr << err;
				throw invalid_argument(err);
			}
			string param;
			size_t param_start = option_end + 1;
			if(param_start < token_size && token[option_end] == ':'){
				param = token.substr(param_start);
			}
			it->second(param);
		}
	}
	string if_name(argv[argc - 1]);
	string buf;
	try {
		read(if_name, buf);
	} catch(runtime_error) {
		string err("Unable to open input file: " + if_name);
		cerr << err;
		throw err;
	}
	Preprocessor p{buf, if_name};
	if(!of_name.size()){
		of_name = if_name + ".out";
	}
	ofstream ofs(of_name, ios::binary);
	ofs.rdbuf()->sputn(&buf[0], buf.size());
	//ofs << buf;
	ofs.close();
}