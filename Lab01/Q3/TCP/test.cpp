#include <algorithm>
#include <bits/stdc++.h>
#include <climits>
#include <cmath>
#include <iostream>
#include <math.h>
#include <queue>
#include <vector>
using namespace std;
typedef long long int ll;
typedef vector<ll> vll;
typedef unsigned long long int ull;
#define LOG(x) cout<<x<<endl
#define log(x) cout<<x<<" "
#define br cout<<endl
#define ping(x) cout<<"ping"<<x<<endl
#define REP(i,a,b) for(ll i=a;i<b;i++)
#define SQ(x) (x)*(x)
const int MOD = (int)1e9 + 7;

char* resulter(char* s){
    std::string str = s;

    std::string s1, s2;
    char op;

    int i=0;
    bool first=true;
    while(i<str.size()){
        if(str[i]=='*' || str[i]=='/' || str[i]=='+' || str[i]=='-' || str[i]=='^'){
            op = str[i];
            i++;
            first=false;
            continue;
        }
        if(first){
            s1.push_back(str[i]);
            i++; continue;
        }else{
            s2.push_back(str[i]);
            i++;
        }
    }
    int i1 = std::stoi(s1);
    int i2 = std::stoi(s2);
    int res=0;
    if(op=='*') res = i1*i2;
    if(op=='/') res = i1/i2;
    if(op=='+') res = i1+i2;
    if(op=='-') res = i1-i2;
    if(op=='^') res = pow(i1, i2);

    std::string result = std::to_string(res);
    char* r = new char(result.size());
    for(int i=0; i<result.size(); i++) r[i]=result[i];
    return r;
}

void solve();

int main(){
    int n;
    cin>>n;
    while(n--) solve();
}

void solve(){
    char* s = "2^2";
    cout<<resulter(s)<<endl;
}