#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <ctime>
#include <cctype>

using namespace std;

class SearchEngine {

public:

	string datasetPath;
	map <string, set<string>> forwardIndex;
	map <string, set<string>> reverseIndex;
	map <string, string> dataset;
	template <class T1, class T2> struct pair;

	SearchEngine(string datasetPath) {
		this->datasetPath = datasetPath;
	}

	string processWord(string word) {

		for (int i = 0, len = word.size(); i < len; i++) {
			if (int(word[i]) <= 255 && int(word[i]) >= -1) {
				if (isupper(word[i]))
					word[i] = tolower(word[i]);
				if (ispunct(word[i])) {
					word.erase(i--, 1);
					len = word.size();
				}
			}
		}

		return word;
	}

	void createForwardIndex() {

		ifstream dataset;								// filestream for corpus dataset
		ofstream forwardIndexStream;
		
		string line;									// variable for parsing lines of dataset
		string heading;
		string body;
		int docCount;

		dataset.open(datasetPath);
		forwardIndexStream.open("forwardIndex.txt");

		if (!dataset) {									// verifying file is open
			cout << "Failed to Open File" << endl;
			exit(1);
		}

		bool trigger = true;
		body = "";
		heading = "";
		docCount = 0;
		while (getline(dataset, line)) {				// distributing data into <heading, body> pairs

			if (line == "") {

				stringstream content(body);
				string word;
				set <string> wordList;

				forwardIndexStream << heading << ":";

				while (content >> word) {

					word = processWord(word);

					if (word.length() > 0) {
						bool check = wordList.insert(word).second;
						if(check)
							forwardIndexStream << word << ";";
					}
				}

				forwardIndexStream << "\n\n";
				forwardIndex[heading] = wordList;

				trigger = true;
				body = "";
			}
			else if (trigger) {

				line = processWord(line);

				heading = line;

				docCount++;
				trigger = false;
			}
			else {
				body.append(line);
			}
		}

		forwardIndexStream << heading << ":";

		stringstream content(body);
		string word;
		set <string> wordList;
		while (content >> word) {
			
			word = processWord(word);

			if (word.length() > 0) {
				bool check = wordList.insert(word).second;
				if (check)
					forwardIndexStream << word << ";";
			}
		}

		forwardIndexStream << "\n\n";
		forwardIndex[heading] = wordList;

		dataset.close();								// closing the filestream
		forwardIndexStream.close();

		cout << docCount << " headings processed." << endl;

	}

	void reverseForward() {
		for (map<string, set<string>>::iterator it = forwardIndex.begin(); it != forwardIndex.end(); it++)
			for (set<string>::iterator itor = it->second.begin(); itor != it->second.end(); itor++)
				reverseIndex[itor->data()].insert(it->first);
	}

	void createReverseIndex() {

		ofstream reverseIndexStream;

		reverseForward();

		reverseIndexStream.open("reverseIndex.txt");

		for (map<string, set<string>>::iterator it = reverseIndex.begin(); it != reverseIndex.end(); it++) {
			reverseIndexStream << it->first << ":";
			for (set<string>::iterator itor = it->second.begin(); itor != it->second.end(); itor++)
				reverseIndexStream << itor->data() << ";";
			reverseIndexStream << "\n\n";
		}

		reverseIndexStream.close();

	}

	void loadForwardIndex() {

		ifstream forwardIndexStream;
		string line;
		string heading;
		set<string> wordList;

		forwardIndexStream.open("forwardIndex.txt");

		if (!forwardIndexStream) {									// verifying file is open
			cout << "Failed to Open File" << endl;
			exit(1);
		}

		while (getline(forwardIndexStream, line)) {				// distributing data into <heading, body> pairs

			if (line != "") {
				stringstream words(line);
				heading = "";
				string word;
				getline(words, word, ':');

				heading = word;

				while (getline(words, word, ';')) {
					forwardIndex[heading].insert(word);
				}
			}
		}

		forwardIndexStream.close();
	}

	void loadReverseIndex() {

		ifstream reverseIndexStream;
		string line;
		string heading;

		reverseIndexStream.open("reverseIndex.txt");

		if (!reverseIndexStream) {									// verifying file is open
			cout << "Failed to Open File" << endl;
			exit(1);
		}

		while (getline(reverseIndexStream, line)) {				// distributing data into <heading, body> pairs

			if (line != "") {
				stringstream words(line);
				heading = "";
				string word;
				getline(words, word, ':');
				heading = word;
				while (getline(words, word, ';')) {
					reverseIndex[heading].insert(word);
				}
			}
		}

		reverseIndexStream.close();
	}

	void loadDataset() {

		ifstream datasetStream;
		string line;
		string heading;
		string body;

		datasetStream.open(datasetPath);

		if (!datasetStream) {									// verifying file is open
			cout << "Failed to Open File" << endl;
			exit(1);
		}

		bool trigger = true;
		body = "";
		heading = "";

		while (getline(datasetStream, line)) {				// distributing data into <heading, body> pairs

			if (line == "") {

				dataset[heading] = body;
				//cout << heading << endl;

				trigger = true;
				body = "";
			}
			else if (trigger) {

				line = processWord(line);

				heading = line;
				trigger = false;
			}
			else {
				body.append(line);
			}
		}

		datasetStream.close();
	}

	void loadIndices() { // load's the forwardindex from file then converts it to reverseindex
		loadForwardIndex();
		reverseForward();
	}

	set<string> search(string keyWord) {

		keyWord = processWord(keyWord);

		return reverseIndex[keyWord];
	}

	void openDoc(string docName) {

		docName = processWord(docName);

		string doc = dataset[docName];
		cout << doc << endl;
	}

};

int main() {

	clock_t start;

	start = clock();
	SearchEngine myEngine = SearchEngine("corpus.txt");

	myEngine.createForwardIndex();
	myEngine.createReverseIndex();
	//myEngine.loadDataset();
	//myEngine.loadReverseIndex();

	cout << "Time Taken: " << (clock() - start) / CLOCKS_PER_SEC << " seconds" << endl;

	/*for (map<string, vector<string>>::iterator it = myEngine.reverseIndex.begin(); it != myEngine.reverseIndex.end(); it++) {
		cout << it->first << " : ";
		for (vector<string>::iterator itor = it->second.begin(); itor != it->second.end(); itor++) {
			cout << itor->data() << " ";
		}
		cout << endl;
	}*/

	while (true) {
		int z;
		cin >> z;

		if (z == 0)
			break;
		else if (z == 1) {
			cout << "Enter search word: ";
			string keyword;
			cin >> keyword;
			set<string> results = myEngine.search(keyword);
			//vector<string> results = myEngine.reverseIndex["alivea"];

			for (set<string>::iterator itor = results.begin(); itor != results.end(); itor++)
				cout << ">> " << itor->data() << endl;
		}
		else if (z == 2) {
			cout << "Enter docname: ";
			string docname;
			cin.ignore();
			getline(cin, docname);
			myEngine.openDoc(docname);
		}
	}
	return 0;
}