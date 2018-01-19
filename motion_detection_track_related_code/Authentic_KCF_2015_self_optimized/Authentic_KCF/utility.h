//file input output string
#include<iostream>
#include<fstream>
#include<cstring>

//functions
#include<cmath>
#include<cstdlib>
#include<cctype>
#include<ctime>
#include<cstddef>

#include<limits>
#include<conio.h>//getch() have a break.
//#include<process.h>

//STL
#include<vector>
#include<list>
#include<deque>
#include<stack>
#include<queue>
#include<set>
#include<map>
/*
#include<multiset>
#include<multimap>
#include<hash_set>
#include<hash_map>
#include<hash_multiset>
#include<hash_multimap>
*/

#include<algorithm>
#include<numeric>
#include<iterator>
#include<functional>

using namespace std;



bool user_says_yes();

bool user_says_yes()
{
	int c;
	bool initial_response=true;
	do
	{
		if(initial_response) cout<<"(y,n)?"<<flush;
		else cout<<"Response with either y or n:"<<flush;
		do
		{
			c=cin.get();
		}while(c=='\n'||c==' '||c=='\t');
		initial_response=false;
	}while(c!='y'&&c!='Y'&&c!='n'&&c!='N');
	return (c=='y'||c=='Y');
}

