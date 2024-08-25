#include<iostream>
#include<fstream>
#include<unordered_map>
#include<queue>
#include<vector>
#include<algorithm>
#include<sstream>
#include<filesystem>
#include<map>
class node{
  public:
    node(char x, unsigned y, node*l = nullptr, node*r = nullptr) : data (x), freq(y), left(l), right(r){}
    char data;
    unsigned freq;
    node*left;
    node*right;
};
class cmp{
  public:
    bool operator () (const node*x, const node*y){return x->freq > y->freq;}
};
void print_node(const node*root){
  if(root == nullptr) return;
  if(not root->left and not root->right)
    std::cout<<root->data;
  print_node(root->left);
  print_node(root->right);
}
node*build_tree(const std::map<char,unsigned>&freq){
  std::priority_queue<node*,std::vector<node*>,cmp>pq;
  for(const auto&i:freq)
    pq.push(new node(i.first,i.second));
    while(pq.size() > 1){
      node*left = pq.top();
      pq.pop();
      node*right = pq.top();
      pq.pop();
      node*n = new node('\0',left->freq + right->freq,left,right);
      n->left = left;
      n->right = right;
      pq.push(n);
    }
    return pq.top();
}
void generate_code(node*root, const std::string&str,std::unordered_map<char,std::string>&huffman){
  if(not root) return;
  if(not root->left and not root->right)
    huffman[root->data] = str;
  generate_code(root->left, str + "0", huffman);
  generate_code(root->right, str + "1", huffman);
}
std::string encode(const std::string & str,std::unordered_map<char,std::string>&huffman){
  std::string encoded;
  for(const auto&i:str)
    encoded+=huffman[i];
  return encoded;
}
std::string decode(node*root,std::string&encoded){
  std::string decoded;
  node*curr = root;
  for(const auto&i:encoded){
    if(i == '1') curr = curr->right;
    else curr = curr->left;
    if(not curr->left and not curr->right){
      decoded += curr->data;
      curr = root;
    }
  }
  return decoded;
}
void write_to_file(node*root,std::ofstream&iFile){
  if(not iFile.is_open()) return;
  if(root == nullptr){
    iFile.put('0');
    return;
  }
  iFile.put('1');
  if(not root->left and not root->right){
    iFile.put('1');
    iFile.put(root->data);
  }else{
    iFile.put('0');
    write_to_file(root->left,iFile);
    write_to_file(root->right,iFile);
  }
}
void compress(){
  std::cout<<"Please enter the path of the file:-"<<std::endl;
  std::string path;
  std::getline(std::cin>>std::ws,path);
  if(not std::filesystem::exists(path) or not std::filesystem::is_regular_file(path)){
    std::cerr<<"Invalid path!"<<std::endl;
    return;
  }
  std::ifstream file(path,std::ios::binary);
  if(not file.is_open()){
    std::cerr<<"Unable to open the file!"<<std::endl;
    return;
  }
  std::string data((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
  std::map<char,unsigned>freq;
  for(const auto&i:data)
    freq[i]++;
  node*root = build_tree(freq);
  std::unordered_map<char,std::string>huffman;
  generate_code(root,"",huffman);
  std::ofstream oFile("compressed.txt",std::ios::binary);
  if(not oFile.is_open()){
    std::cerr<<"Unable to create the compressed file!"<<std::endl;
    return;
  }
  std::size_t size = freq.size();
  oFile.write(reinterpret_cast<char*>(&size),sizeof(std::size_t));
  oFile.put('\n');
  for(const auto& i : freq) {
    oFile.write(reinterpret_cast<const char*>(&i.first), sizeof(char));
    oFile.write(reinterpret_cast<const char*>(&i.second), sizeof(unsigned));
}
  oFile.put('\n');
  std::string encoded_str  = encode(data,huffman);
  char paddingBits = 8 - (encoded_str.size() % 8);
  if (paddingBits == 8) paddingBits = 0;
    oFile.write(&paddingBits, sizeof(char));
  oFile.put('\n');
  for(int i = 0; i < encoded_str.size(); i+=8){
    char byte = 0;
    for(int j = 0; j < 8 and (i+j) < encoded_str.size(); j++){
      byte <<= 1;
      if(encoded_str[i+j] == '1') byte |= 1;
    }
    oFile.write(&byte,sizeof(char));
  }
  std::cout<<"The frequencies of the elements are as follows:-"<<std::endl;
    for(const auto&i:freq)
        std::cout<<i.first<<'\t'<<i.second<<std::endl;
    std::cout<<"The huffman tree of the text is as follows:-"<<std::endl;
    for(const auto&i:huffman)
      std::cout<<i.first<<'\t'<<i.second<<std::endl;
    std::cout<<"Printing out the root node:-"<<std::endl;
    print_node(root);
    std::cout<<std::endl;
  oFile.close();
  file.close();
  std::cout<<"Compression is done!"<<std::endl;
}
void decompress(){
    std::cout<<"Please enter the path of the compressed file:-"<<std::endl;
    std::string path;
    std::getline(std::cin>>std::ws,path);
    if(not std::filesystem::is_regular_file(path) or not std::filesystem::exists(path)){
        std::cerr<<"Invalid path!"<<std::endl;
        return;
    }
    std::ifstream iFile(path,std::ios::binary);
    if(not iFile.is_open()){
        std::cerr<<"Unable to open the file!"<<std::endl;
        return;
    }
    std::ofstream oFile("decompressed.txt",std::ios::binary);
    if(not oFile.is_open()){
        std::cerr<<"Unable to create the decompressed file!"<<std::endl;
        return;
    }
    std::size_t size;
    iFile.read(reinterpret_cast<char*>(&size),sizeof(std::size_t));
    std::map<char,unsigned>freq;
    char ch;
    iFile.get(ch);
    char data;
    unsigned freqs;
    for(size_t i = 0; i < size; i++) {
      iFile.read(reinterpret_cast<char*>(&data), sizeof(char));
      iFile.read(reinterpret_cast<char*>(&freqs), sizeof(unsigned));
      freq[data] = freqs;
    }
    node*root = build_tree(freq);
    std::unordered_map<char,std::string>huffman;
    generate_code(root,"",huffman);
    iFile.get(ch);
    char paddingBits;
    iFile.read(&paddingBits, sizeof(char));
    iFile.get(ch);
    std::cout<<"The frequencies of the elements are as follows:-"<<std::endl;
    for(const auto&i:freq)
        std::cout<<i.first<<'\t'<<i.second<<std::endl;
    std::cout<<"The huffman tree of the text is as follows:-"<<std::endl;
    for(const auto&i:huffman)
      std::cout<<i.first<<'\t'<<i.second<<std::endl;
    std::cout<<"Printing out the root node:-"<<std::endl;
    print_node(root);
    std::cout<<std::endl;
    char bit;
    node*curr = root;
    while(iFile.get(bit)){
        int limit = (iFile.peek() == EOF) ? (8 - paddingBits) : 8;
        for(int i = 7; i >= 8 - limit; i--){
            if((bit >> i) & 1) curr = curr->right;
            else curr = curr->left;
            if(not curr->left and not curr->right){
                oFile.put(curr->data);
                curr = root;
            } 
        }
    }
    iFile.close();
    oFile.close();
    std::cout<<"Decompression done!"<<std::endl;
}
void node_deleter(node*root){
  if(root == nullptr) return;
  node_deleter(root->left);
  node_deleter(root->right);
  delete root;
}
int main(){
  std::cout<<"Please choose either compress or decompress a file:-"<<std::endl;
  std::cout<<"A - Compress"<<std::endl;
  std::cout<<"B - Decompress"<<std::endl;
  char ch;
  std::cin>>ch;
  if(ch == 'a') compress();
  else if (ch == 'b') decompress();
  std::cout<<"Press Enter to clost this window..."<<std::endl;
  std::cin.get();
  return EXIT_SUCCESS;
}
