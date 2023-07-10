#include "json.hpp"
#include<iostream>
#include<vector>
#include<map>
#include<string>
using namespace std;
using json = nlohmann::json;
// json 序列化 example
// 容器序列化
string func1()
{
    json js;
    js["msg"]={{"first","begin"},{"second","last"}};
    js["from"]="a";
    js["to"]="b";
    //容器序列化
    vector<int>vec={1,2,3};
    map<int,string>m;
    m[1]={"x"};
    m[2]={"y"};
    m[3]={"z"};
    js["path"]=m;
    js["num"]=vec;
    string sendBuf = js.dump();
    cout<<sendBuf.c_str()<<endl;
    return sendBuf;
};
//json 反序列化
void func2()
{
    string recvBuf=func1();
    // stirng to json object
    json jsBuf = json::parse(recvBuf);
    cout<<jsBuf["msg"];
};
int main()
{
    //func1();
    func2();
    
    return 0;
}