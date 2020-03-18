#ifndef __CSV_LEADER_H__
#define __CSV_LEADER_H__

#include <iostream>
#include <vector>
#include <string>

using namespace std;
using Row = vector<string>;
using Table = vector<Row>;

class CSVReader
{
private:
	static void readRow(istream &_file, Row &out_row);
	static void writeRow(ofstream &out_file, Row _row);

public:
	CSVReader()  {}
	Table table;

	bool read(string fileName);
	bool write(string fileName);
	int xCount();	// �� ����(���� ����)
	int yCount();		// �� ����(���� ����)
	
	string getString(int x, int y) { return table[y][x]; }
	double getDouble(int x, int y) { return stod(table[y][x]); }
	float getFloat(int x, int y) { return stof(table[y][x]); }
	int getInt(int x, int y) { return stoi(table[y][x]); }
	long getLong(int x, int y) { return stol(table[y][x]); }
	long long getLongLong(int x, int y) { return stoll(table[y][x]);}
};

#endif