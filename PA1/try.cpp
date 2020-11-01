#include<iostream>
#include<vector>
using namespace std;
int main(){
    vector<int> a;
    cout << a.size();
    a.push_back(12);
    cout << a.size()<<a.at(0);
    return 0;
}