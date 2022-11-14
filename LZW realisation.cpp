#include <unordered_map>
#include <iostream>
#include <vector>
#include <string>
#include <locale.h>
#include <Windows.h>

using namespace std;

class ConsoleCP
{
	int oldin;
	int oldout;

public:
	ConsoleCP(int cp)
	{
		oldin = GetConsoleCP();
		oldout = GetConsoleOutputCP();
		SetConsoleCP(cp);
		SetConsoleOutputCP(cp);
	}

	~ConsoleCP()
	{
		SetConsoleCP(oldin);
		SetConsoleOutputCP(oldout);
	}
};

vector<int> encoding(string input_stream) {
	setlocale(LC_ALL, "rus");
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	vector<int> output_stream;
	cout << "<< ÊÎÄÈÐÎÂÀÍÈÅ >>\n";
	unordered_map<string, int> dictionary;
	int code = 0;
	for (code = 0; code < 256; code++) {
		string adding_string = "";
		adding_string += char(code);
		dictionary[adding_string] = code;
		//cout << adding_string << "\t" << code << "\n";
	}

	string W = "";
	W += input_stream[0];
	string K = "";

	cout << "Ñòðîêà\t| Íà âûõîä\t| Äîáàâëåíèå\n";
	cout << "________|_______________|_____________________\n";
	//cout << "Íîâàÿ ñòðîêà\tÊîä\tÂûâîä\n";

	for (int i = 0; i < input_stream.length(); i++) {
		if (i != input_stream.length() - 1) {
			K += input_stream[i + 1];
		}
		if (dictionary.find(W + K) != dictionary.end()) {
			W += K;
			K = "";
		}
		else {
			cout << W << "\t|\t" << dictionary[W] << "\t| " << W + K << "\t\t" << code << "\n";

			dictionary[W + K] = code;
			output_stream.push_back(dictionary[W]);
			code++;
			W = K;
		}
		K = "";
	}
	cout << W << "\t|\t" << dictionary[W] << "\t| " << W + K << "\t\t" << code << "\n";

	output_stream.push_back(dictionary[W]);
	return output_stream;
}
 vector<string> decoding(vector<int> coded_string){
	 vector<string> decoded_string;
	 int code;
	 cout << "\n\n<< ÄÅÊÎÄÈÐÎÂÀÍÈÅ >>\n";
	 unordered_map<int, string> dictionary;

	 cout << "Ñòðîêà\t| Íà âûõîä\t| Äîáàâëåíèå\n";
	 cout << "________|_______________|_____________________\n";

	 for (code = 0; code < 256; code++) {
		string adding_string = "";
		adding_string += char(code);
		dictionary[code] = adding_string;
	 }
	 int W = coded_string[0];
	 int K;
	 string phrase = dictionary[W];
	 string symbol = "";
	 symbol += phrase[0];
	 decoded_string.push_back(phrase);

	 for (int i = 0; i < coded_string.size() - 1; i++) {
		 K = coded_string[i + 1];
		 if (dictionary.find(K) == dictionary.end()) {
			 phrase = dictionary[W];
			 phrase = phrase + symbol;
		 }
		 else {

			 phrase = dictionary[K];
			 cout << phrase << "\t|\t" << K << "\t|" << symbol + phrase << "\t\t" << code << "\n";

		 }
		 decoded_string.push_back(phrase);
		 symbol = "";
		 symbol += phrase[0];
		 dictionary[code] = dictionary[W] + symbol;
		 code++;
		 W = K;

	 }
	 cout << phrase << "\t|\t" << K << "\t|" << phrase << "\t\t" << code << "\n";

	 return decoded_string;
 }


int main() {
	ConsoleCP cp(1251);
	string input_stream;
	getline(cin, input_stream, '#');
	vector<int> coded = encoding(input_stream);
	cout << "ÇÀÊÎÄÈÐÎÂÀÍÍÎÅ ÑÎÎÁÙÅÍÈÅ: ";
	for (int i = 0; i < coded.size(); i++) {
		cout << coded[i] << ' ';
	}
	vector<string> decoded = decoding(coded);
	cout << "ÄÅÊÎÄÈÐÎÂÀÍÍÎÅ ÑÎÎÁÙÅÍÈÅ: ";
	for (int i = 0; i < decoded.size(); i++) {
		cout << decoded[i];
	}
	cout << "\n";
}
