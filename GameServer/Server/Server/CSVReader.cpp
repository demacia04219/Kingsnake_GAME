#include "CSVReader.h"
#include <ostream>
#include <istream>
#include <fstream>
#include <sstream>

void CSVReader::readRow(istream &_file, Row &out_row)
{
	stringstream ss;
	bool isInquotes = false;

	while (_file.good())
	{
		char c = _file.get();
		if (!isInquotes && c == '"')
		{
			isInquotes = true;
		}
		else if (isInquotes && c == '"')
		{
			if (_file.peek() == '"')
			{
				ss << (char)_file.get();
			}
			else
			{
				isInquotes = false;
			}
		}
		else if (!isInquotes && c == ',')
		{
			out_row.push_back(ss.str());
			ss.str("");
		}
		else if (!isInquotes && (c == '\r' || c == '\n'))
		{
			if (_file.peek() == '\n') { _file.get(); }
			out_row.push_back(ss.str());
			break;
		}
		else
		{
			ss << c;
		}
	}
}
void CSVReader::writeRow(ofstream &out_file, Row _row)
{
	int columnLength = static_cast<int>(_row.size());
	for (int idx_column = 0; idx_column < columnLength; idx_column++)
	{
		if (_row[idx_column].find(',', 0) >= _row[idx_column].size())
			out_file << _row[idx_column];
		else
			out_file << '"' + _row[idx_column] + '"';
			

		if (idx_column + 1 != columnLength)
		{
			out_file << ",";
		}
	}
	out_file << "\n";
}
bool CSVReader::read(string fileName) {
	ifstream file(fileName);

	table.clear();

	if (file.fail())  //만약 bad() 함수가 실패 하면..
	{
		return false;
	}
	while (file.good())
	{
		vector<string> row;
		readRow(file, row);
		table.push_back(row);
	}

	file.close();
	return true;
}
bool CSVReader::write(string fileName) {
	ofstream file(fileName);

	if (file.fail())
	{
		return false;
	}
	for (auto row : table)
	{
		writeRow(file, row);
	}

	file.close();

	return true;
}
int CSVReader::xCount()
{
	return table[0].size();
}
int CSVReader::yCount()
{
	return table.size();
}