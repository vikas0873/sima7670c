#include <iostream>
#include <vector>
#include <string.h>
using namespace std;

void reverseString(string &str)
{
	int n = str.length()-1;
	for (int i = 0; i < str.length() / 2; i++)
	{
		char temp = str[i];
		str[i] = str[n - i];
		str[n - i] = temp;
	}
}
int main()
{
	string s = "vikas";
	reverseString(s);
	cout << s ;
	return 0;
}