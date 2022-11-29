#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include "assert.h" 
#include <map>
#include <set>
#include <algorithm>
#include <iomanip>

using namespace std;
/*
nvtxs : number of vertex

nhedges : number of hyperedges

vwgts :  An array of size nvtxs that stores the weight of the vertices, ex weight of vertex i is stored at vwgts[i]

-----describe graph-----
eptr : 
size = nhedges+1
used to index the eind

eind :
-----describe graph-----

hewgts : An array of size nhedges that stores the weight of the hyperedges. The weight of the i hyperedge is stored
at location hewgts[i]. If the hyperedges in the hypergraph are unweighted, then hewgts can be NULL.

nparts : The number of desired partitions.

ubfactor : This is the relative imbalance factor to be used at each bisection step. Its meaning is identical to the
UBfactor parameter of shmetis, and hmetis described in Section 3.

options :

part : This is an array of size nvtxs that returns the computed partition. Specifically, part[i] contains the partition
number in which vertex i belongs to. Note that partition numbers start from 0.

edgecut : This is an integer that returns the number of hyperedges that are being cut by the partitioning algorithm.

*/
extern "C" void HMETIS_PartRecursive (int nvtxs, int nhedges, int *vwgts, int *eptr, int *eind, int *hewgts, int nparts,
int ubfactor, int *options, int *part, int *edgecut);


class Graph{
public:
    //map<string, set<string>> adjList_;
    map<string, vector<string>> adjList_;
    map<string, vector<string>> re_adjList_;
    //map<dest, set<src>> adjList_;
    Graph(){}

    Graph(vector<string> src,vector<vector<string>> dest){
        for(int i=0;i<src.size();i++){
            adjList_.insert( make_pair(src[i],dest[i]) );
        }
    }

    void insert_edge(string src,string dest){
        adjList_[dest].push_back(src);
    }
    void insert_edge2(string src,string dest){
        re_adjList_[src].push_back(dest);
    }

    void print_graph(vector<string> dest){
        for(int i=0;i<dest.size();i++){
            cout << "dest: " << dest[i]<< ", src={ ";
            for(auto &s:adjList_[dest[i]]){
                cout << s << " ";
            }
            cout <<"}"<< endl;
        }
    }
    void print_graph2(vector<string> src){
        for(int i=0;i<src.size();i++){
            cout << "src: " << src[i]<< ", dest={ ";
            for(auto &s:re_adjList_[src[i]]){
                cout << s << " ";
            }
            cout <<"}"<< endl;
        }
    }

};

void split_str(string s,vector<string> &s_vect){
    while (1){
        s_vect.push_back(s.substr(0, s.find(" ")));
        s = s.substr(s.find(" ")+1,s.length());
        if (s.find(" ") == -1){
            s_vect.push_back(s);
            break;
        }
    }
}

template<typename T>
void pop_front(std::vector<T>& vec)
{
    assert(!vec.empty());
    vec.erase(vec.begin());
}



int main(int argc, char *argv[]){
    string line;
    string Filename = argv[1];
    string ratio_ = argv[2];
    double ratio = stof(ratio_);
    vector<string> blif_everyline_data;
    ifstream myFile;
    myFile.open(Filename);

    string model_;
    string input_;
    string output_;
    string result=""; //存放結果
    vector<string> input_vec;
    vector<string> output_vec;
    vector<string> name_vec; //存放每一行name

    while(getline(myFile, line)){
        blif_everyline_data.push_back(line);
    }
    
    myFile.close();

    model_ = blif_everyline_data[0];
    input_ = blif_everyline_data[1];
    output_ = blif_everyline_data[2];


    int first_name = 0;
    for(int i=0;i<blif_everyline_data.size();i++){
        if(blif_everyline_data[i].find(".names")!=-1){
            if(first_name==0){
                first_name = i;//紀錄第一個name的位置 
            }
            name_vec.push_back(blif_everyline_data[i]);
        }        
    }

    //cout << "first name " << first_name <<endl;
    //處理input output字串
    split_str(input_,input_vec);
    pop_front(input_vec);
    split_str(output_,output_vec);
    pop_front(output_vec);

    // ----------Graph area---------

    Graph mygraph;
    vector<string> dest_vec; //放dest以便print
    vector<string> src_vec;
    vector<string> node_vec; //放所有的node 等等要建hash table

    for(auto &v:name_vec){
        vector<string> name_test; //暫時放src
        vector<string> name_test2;
        split_str(v,name_test);
        pop_front(name_test);

        name_test2 = name_test;

        string dest;
        dest = name_test.back();
        name_test.pop_back();
        for(auto &src:name_test){
            mygraph.insert_edge(src,dest);
        }
        dest_vec.push_back(dest);


        string dest2;
        dest2 = name_test2.back();

        vector<string>::iterator it_ = find(node_vec.begin(), node_vec.end(), dest2);
        if (it_ == node_vec.end()) node_vec.push_back(dest2);

        name_test2.pop_back();
        for(auto &src:name_test2){
            mygraph.insert_edge2(src,dest2);

            vector<string>::iterator it = find(src_vec.begin(), src_vec.end(), src); // find src
            if (it == src_vec.end()) src_vec.push_back(src);

            vector<string>::iterator it2 = find(node_vec.begin(), node_vec.end(), src); // find src
            if (it2 == node_vec.end()) node_vec.push_back(src);

        }

    }

    //打印graph關係圖
    //mygraph.print_graph(dest_vec);
    //mygraph.print_graph2(src_vec);

    int nvtxs = 0; // number of vertex
    nvtxs = dest_vec.size();
    //cout << "nvtxs: " << nvtxs << endl;

    //計算hyper edge
    int nhedges = 0; //number of hyperedge

    nhedges = mygraph.re_adjList_.size() - input_vec.size();   //有幾個src就是幾個hyperedge
    //---------建立eptr eind----------
    vector<int> eptr;
    vector<int> eind;


    map<string,int> node_dict;
    int index_dict=0;
    for(auto &v:node_vec){
        vector<string>::iterator itdict = find(input_vec.begin(), input_vec.end(), v);
        if(itdict!=input_vec.end()){
            continue;
        }
        node_dict[v] = index_dict;
        index_dict++;
    }
    // for(auto &v:node_dict){
    //     cout << v.first << " " << v.second << endl;
    // }
    map<int,string> re_node_dict;
    for(auto &v:node_dict){
        re_node_dict[v.second] = v.first;
    }

    int index=0;
    for(auto &s:mygraph.re_adjList_){

        vector<string>::iterator it3 = find(input_vec.begin(), input_vec.end(), s.first);
        if (it3 != input_vec.end()) {
            continue;
        }

        eind.push_back(node_dict[s.first]);
        eptr.push_back(index);
        index++;
        for(auto &d:s.second){
            eind.push_back(node_dict[d]);
            index++;
        }
    }
    eptr.push_back(index);

    // cout << eptr.size() << endl;
    // for(int i=0;i<eptr.size();i++){
    //     cout << i << ":" << eptr[i] <<endl;
    // }

    //---------epind eptr完成------------

    //---------vwgts---------
    vector<string> inter_node;
    for(auto &v:node_vec){
        vector<string>::iterator it4 = find(input_vec.begin(), input_vec.end(), v);
        if (it4 != input_vec.end()) {
            continue;
        }
        inter_node.push_back(v);
    }

    //store boolean function
    vector<vector<string>> boolean_function;
    vector<string> every_dest_boolean;
    for(int i=first_name+1;i<blif_everyline_data.size();i++){
        if(blif_everyline_data[i].find(".names")!=-1 || blif_everyline_data[i].find(".end")!=-1){
            boolean_function.push_back(every_dest_boolean);
            every_dest_boolean.clear();
            continue; //跳到name下一行
        }
        else{
            every_dest_boolean.push_back(blif_everyline_data[i]);
        }
    }
    //把boolean_funcion的空白以及最後的0/1保存下來
    for(auto &des:boolean_function){
        for(auto &lines:des){
            char temp_str = lines.back();
            lines.pop_back();
            lines.pop_back();
            lines = lines + temp_str;
        }
    }


    //create a hash table for dest and boolean fuction
    map<string,int> hash_index;
    for(int i=0;i<dest_vec.size();i++){
        hash_index.insert(make_pair(dest_vec[i],i));
    }

    vector<int> vwgts(nvtxs,0);

    for(auto &v:node_dict){
        vwgts[v.second] = boolean_function[hash_index[v.first]].size() + mygraph.adjList_[v.first].size();
    }

    //---------vwgts end-----

    //---------hewgts--------
    vector<int> hewgts(nhedges,0);
    for(auto &v:node_dict){
        hewgts[v.second] = mygraph.re_adjList_[v.first].size();
    }
    // for(int i=0;i<hewgts.size();i++){
    //     cout << i << " " << hewgts[i] << endl;
    // }
    //---------hewgts end-----

    //---------ubfactor-------
    int ubfactor = abs(((50-ratio)*nvtxs)/100);
    //---------ubfactor end---

    //---------option---------
    int options[9] = {0,0,0,0,0,0,0,0,0};
    //---------option end-----
    int cut = 0;
    vector<int> part(nvtxs,0);
    HMETIS_PartRecursive(nvtxs,nhedges,vwgts.data(),eptr.data(),eind.data(),hewgts.data(),2,ubfactor,options,part.data(),&cut);
    // cout << cut << endl;
    // for(auto &i:part){
    //     cout << i << endl;
    // }

    int Area_X = 0;
    for(int i=0;i<part.size();i++){
        if(part[i]==0) continue;
        Area_X = Area_X + vwgts[i];
    }
    int Area_Y = 0;
    for(int i=0;i<part.size();i++){
        if(part[i]==1) continue;
        Area_Y = Area_Y + vwgts[i];
    }
    double ratio_x = (double)Area_X / ((double)Area_X+(double)Area_Y);
    double ratio_y = (double)Area_Y / ((double)Area_X+(double)Area_Y);



    cout << "Partition X" << endl;
    cout << "Nodes: ";
    for(int i=0;i<part.size();i++){
        if(part[i]==0) continue;
        cout << re_node_dict[i] << " ";
    }
    cout << endl;
    cout << "Area: ";
    cout << Area_X << endl;

    cout << "Ratio: ";
    cout << setprecision(3)<<ratio_x << endl;
    cout << "Partition Y" << endl;
    cout << "Nodes: ";
    for(int i=0;i<part.size();i++){
        if(part[i]==1) continue;
        cout << re_node_dict[i] << " ";
    }
    cout << endl;
    cout << "Area: ";
    cout << Area_Y << endl;
    cout << "Ratio: ";
    cout << setprecision(3)<<ratio_y << endl;
    cout << "Cut size: " << cut << endl;
    cout << "END" << endl;
    return 0;
}