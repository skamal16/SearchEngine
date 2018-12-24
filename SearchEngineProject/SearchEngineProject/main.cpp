#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <ctime>
#include <cctype>
#include <utility>
#include <algorithm>

using namespace std;

class Hit {
public:
	mutable int count;
	string word;

	Hit(string word) {
		this->word = word;
		this->count = 1;
	}

	bool operator<(const Hit& rhs) const {
		return word < rhs.word;
	}
};

class Doc {
public:
	int score;
	string docTitle;
	
	Doc(string title, int count) {
		this->docTitle = title;
		this->score = count;
	}

	bool operator<(const Doc& rhs) const {
		return docTitle < rhs.docTitle;
	}
};

class Result {
public:
	int score;
	string docTitle;

	Result(Doc doc) {
		this->score = doc.score;
		this->docTitle = doc.docTitle;
	}

	bool operator<(const Result& rhs) const {
		return score > rhs.score;
	}
};

set<Doc> copy(set<Doc>::iterator first, set<Doc>::iterator last, set<Doc> result) {

	while (first != last) {
		result.insert(*first);
		++first;
	}
	return result;
}
set<Doc> set_union(set<Doc> first, set<Doc> second) {

	set<Doc> result;

	set<Doc>::iterator first1 = first.begin();
	set<Doc>::iterator last1 = first.end();
	set<Doc>::iterator first2 = second.begin();
	set<Doc>::iterator last2 = second.end();

	while (true) 
	{

		if (first1 == last1) return copy(first2, last2, result);
		if (first2 == last2) return copy(first1, last1, result);

		if (*first1 < *first2) {
			result.insert(*first1); 
			++first1; 
		}
		else if (*first2 < *first1) { 
			result.insert(*first2);
			++first2; 
		}
		else { 
			Doc doc = Doc(first1->docTitle, 2*(first1->score + first2->score));
			result.insert(doc);
			++first1; ++first2; 
		}
	}
}

class SearchEngine {

public:

	string datasetPath;
	map <string, set<Hit>> forwardIndex;
	map <string, set<Doc>> reverseIndex;
	map <string, string> dataset;

	SearchEngine(string datasetPath) {
		this->datasetPath = datasetPath;
	}

	static string processWord(string word) {

		for (int i = 0, len = word.size(); i < len; i++) {
			if (int(word[i]) <= 255 && int(word[i]) >= -1) {
				if (isupper(word[i]))
					word[i] = tolower(word[i]);
				if (ispunct(word[i]) || word[i] == ' ') {
					word.erase(i--, 1);
					len = word.size();
				}
			}
		}

		return word;
	}

	void createForwardIndex() {

		cout << "creating forward index" << endl;

		ifstream dataset;								// filestream for corpus dataset
		
		string line;									// variable for parsing lines of dataset
		string heading;
		string body;
		int docCount;

		dataset.open(datasetPath);

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
				set <Hit> wordList;

				while (content >> word) {

					word = processWord(word);

					if (word.length() > 0) {

						//Hit hit = Hit(word);

						pair<set<Hit>::iterator, bool> check;

						check = wordList.insert(word);
						if (!check.second) {
							check.first->count++;
						}
					}
				}

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

		stringstream content(body);
		string word;
		set <Hit> wordList;
		while (content >> word) {
			
			word = processWord(word);

			if (word.length() > 0) {
				pair<set<Hit>::iterator, bool> check;

				check = wordList.insert(word);
				if (!check.second) {
					check.first->count++;
				}
			}
		}

		forwardIndex[heading] = wordList;

		dataset.close();								// closing the filestream

		cout << "forward index created with: " << docCount << " articles." << endl;

	}

	void saveForwardIndex() {

		cout << "writing forward index to file" << endl;

		ofstream forwardIndexStream;

		forwardIndexStream.open("forwardIndex.txt");

		for (map<string, set<Hit>>::iterator it = forwardIndex.begin(); it != forwardIndex.end(); it++) {
			forwardIndexStream << it->first << ":";
			for (set<Hit>::iterator itor = it->second.begin(); itor != it->second.end(); itor++)
				forwardIndexStream << itor->word << "-" << itor->count << ";";
			forwardIndexStream << "\n\n";
		}
		forwardIndexStream.close();

		cout << "finished writing forward index to file" << endl;
	}

	void createReverseIndex() {

		cout << "creating reverse index" << endl;

		for (map<string, set<Hit>>::iterator it = forwardIndex.begin(); it != forwardIndex.end(); it++)
			for (set<Hit>::iterator itor = it->second.begin(); itor != it->second.end(); itor++) {
				Doc doc = (itor->word == it->first) ? Doc(it->first, itor->count * 10) : Doc(it->first, itor->count);
				reverseIndex[itor->word].insert(doc);
			}

		cout << "finished creating reverse index" << endl;
	}

	void saveReverseIndex() {

		cout << "writing reverse index to file" << endl;

		ofstream reverseIndexStream;

		reverseIndexStream.open("reverseIndex.txt");

		for (map<string, set<Doc>>::iterator it = reverseIndex.begin(); it != reverseIndex.end(); it++) {
			reverseIndexStream << it->first << ":";
			for (set<Doc>::iterator itor = it->second.begin(); itor != it->second.end(); itor++)
				reverseIndexStream << itor->docTitle << "-" << itor->score << ";";
			reverseIndexStream << "\n\n";
		}
		reverseIndexStream.close();

		cout << "finished writing reverse index to file" << endl;
	}

	void loadForwardIndex() { //depreciated

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

					stringstream pair(word);
					string title;
					string score;
					getline(pair, title, '-');
					getline(pair, score, '-');
					Doc doc = Doc(title, stoi(score));
					reverseIndex[heading].insert(doc);
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

	set<Doc> combine(vector<string> keyWords) {

		if (keyWords.size() == 1) {
			return reverseIndex[keyWords[0]];
		}
		
		vector<string> res(keyWords.size() - 1);
		copy(keyWords.begin() + 1, keyWords.end(), res.begin());

		return set_union(reverseIndex[keyWords[0]], combine(res));
	}

	set<Doc> search(string query) {
		vector<string> keyWords;

		stringstream queries(query);
		string keyWord;

		while (getline(queries, keyWord, ','))
			keyWords.push_back(keyWord);

		vector<string> processedWords(keyWords.size());
		transform(keyWords.begin(), keyWords.end(), processedWords.begin(), [](string i) {return SearchEngine::processWord(i);});

		return combine(processedWords);
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

	//myEngine.createForwardIndex();
	//myEngine.createReverseIndex();
	//myEngine.saveForwardIndex();
	//myEngine.saveReverseIndex();
	myEngine.loadDataset();
	myEngine.loadReverseIndex();

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
			cout << "Enter search words separated by comma: ";
			string query;
			cin.ignore();
			getline(cin, query);
			set<Doc> results = myEngine.search(query);

			vector<Result> res;

			for (set<Doc>::iterator itor = results.begin(); itor != results.end(); itor++)
				res.push_back(*itor);

			sort(res.begin(), res.end());

			z = 1;

			vector<Result>::iterator it = res.begin();

			while (z == 1) {
				vector<Result>::iterator itor = it + 10;
				while(it != res.end() && it != itor){
					cout << ">> " << it->docTitle << "-" << it->score << endl;
					it++;
				}

				cin >> z;

			}
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