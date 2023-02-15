#include <iostream>
#include <unordered_map>
#include <fstream>

using namespace std;

bool is_equal_files(char* file1_path, char* file2_path) {

    ifstream file1(file1_path, ios::binary);
    ifstream file2(file2_path, ios::binary);

    if (file1 && file2) {

        char ch1, ch2;
        bool result = true;

        while (file1.get(ch1) && file2.get(ch2)) {

            if (ch1 != ch2) {
                result = false;
                break;
            }
        }
        if (result) {
            return true;
        }
        else {
            return false;
        }
    }
    else
        cout << "Error opening file!" << endl;
        return false;
}

void encoding(char *in_file_path, char *out_file_path) {

    ifstream in(in_file_path);
    ofstream out(out_file_path);
    // out << in_file_path;

    if (in) {
        // инициализация словаря <фраза, код>
        unordered_map<string, int> dictionary; 

        // заполнение первых 256 строк словаря символами по таблице ascii
        for (int i = 0; i <= 255; i++) {
            string single_character (1, (char)i);
            dictionary[single_character] = i;
            // out << single_character << "\t" << i << endl;
        }    

        // номер следующего незанятого кода
        int code = 256;

        char next_character;
        string phrase = "";

        // кодирование
        while (!in.eof()) {
            next_character = in.get();
            // out << next_character;
            if (dictionary.find(phrase + next_character) != dictionary.end()) {
                phrase += next_character;
            }
            else {
                // out << dictionary[phrase];
                out << dictionary[phrase] << "\n";
                dictionary[phrase + next_character] = code;
                // out << phrase + next_character << "\t" << code << endl;
                code++;
                phrase = next_character;
            }
        }
    }
    in.close();
    out.close();
}

void decoding(char *in_file_path, char *out_file_path) {

    ifstream in(in_file_path);
    ofstream out(out_file_path);

    if (in) {
        // инициализация словаря <фраза, код>
        unordered_map<int, string> dictionary; 

        // заполнение первых 256 строк словаря символами по таблице ascii
        for (int i = 0; i <= 255; i++) {
            string single_character (1, (char)i);
            dictionary[i] = single_character;
            // out << single_character << "\t" << i << endl;
        }    
        // номер следующего незанятого кода
        int code = 256;

        // декодирование
        int existing_code;
        in >> existing_code;
        // out << existing_code << " ";

        int new_code = 0;
        string phrase = dictionary[existing_code];
        string previous_character = "";
        previous_character += phrase[0];

        out << dictionary[existing_code];
        // out << dictionary[existing_code] << " ";
        
        while (in >> new_code) {
            // out << new_code << " ";
            if (dictionary.find(new_code) == dictionary.end()) {
                phrase = dictionary[existing_code];
                phrase += previous_character;
            }
            else {
                phrase = dictionary[new_code];
            }
            // out << phrase << " ";
            out << phrase;
            previous_character = "";
            previous_character += phrase[0];
            dictionary[code] = dictionary[existing_code] + previous_character;
            code++;
            existing_code = new_code;
        }
    } 
    in.close();
    out.close();  
}

int main() {

    // тестовая система
    int current_test = 0;
    int successfull_tests = 0;
    int failed_tests = 0;
    
    for (int i = 0; i <= 10; i++) {

        char in_file_path[100];
        char encoded_file_path[100];
        char decoded_file_path[100];

         // sprintf(in_file_path, "/Tests/in/%i.txt", current_test);
        sprintf(in_file_path, "D:/Desktop/LZW algorithm/LZW-algorithm/Tests/in/%i.txt", current_test);
        sprintf(encoded_file_path, "D:/Desktop/LZW algorithm/LZW-algorithm/Tests/encoded/%i.txt", current_test);
        sprintf(decoded_file_path, "D:/Desktop/LZW algorithm/LZW-algorithm/Tests/decoded/%i.txt", current_test);
        // cout << in_file_path << "\n";

        encoding(in_file_path, encoded_file_path);
        decoding(encoded_file_path, decoded_file_path);

        // double original_file_size = 0;
        // double compressed_file_size = 0;

        // fstream original_file(in_file_path);
        // fstream compressed_file(encoded_file_path);

        // original_file.seekg (0, ios::end);
        // original_file_size = original_file.tellg();

        // compressed_file.seekg (0, ios::end);
        // compressed_file_size = compressed_file.tellg();

        // double compression_ratio = original_file_size / compressed_file_size;

        // original_file.close();
        // compressed_file.close();

        if (is_equal_files(in_file_path, decoded_file_path)) {
            cout << "test" << current_test << " - success" << "\n";
            // cout << "original file size: " << original_file_size << "byte" << "\n";
            // cout << "compressed file size: " << compressed_file_size << "byte" << "\n";
            // cout << "compression ratio - " << compression_ratio << "\n";
            successfull_tests++;
        }
        else {
            cout << "test" << current_test << " -  fail" << "\n";
            failed_tests++;
        }
        current_test++;
    }
    cout << "successfull tests: " << successfull_tests << "\n";
    cout << "failed tests: " << failed_tests << "\n";

    return 0;
}