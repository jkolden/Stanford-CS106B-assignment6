/*
John Kolden, SCPD
Standford University CS106B
Filename: encoding.cpp
Assignment 6
May 22, 2015
Section Leader: Sarah Spikes

Purpose of Program:
This is a program that performs encoding and decoding of files. It uses the
Huffman Compression Algorithm to reduce the number of bits required to store
commonly used characters.
*/

#include "encoding.h"
#include "pqueue.h"
#include "bitstream.h"
#include "strlib.h"
#include "filelib.h"

void buildEncodingTreeHelper(PriorityQueue<HuffmanNode*>&);
void buildEncodingMapHelper(HuffmanNode*, Map<int, string>&, string);
void writeBitsToOutput(string, obitstream&);
void decodeDataHelper(ibitstream&,HuffmanNode*,HuffmanNode*, ostream&, string ) ;

//this function builds a map of characters as keys and the number of times
//they appear in the given file as values.
Map<int, int> buildFrequencyTable(istream& input) {

    Map<int, int> freqTable;
    int ch = input.get();

    while (ch != EOF) {
        if (freqTable.containsKey(ch)) {
            int value = freqTable.get(ch);
            freqTable.put(ch, value + 1);
        } else {
            freqTable.add(ch, 1);
        }

        ch = input.get();
    }

    freqTable.put(PSEUDO_EOF, 1);

    return freqTable;
}

//this function build a priority queue from the frequency map,
//and then removes nodes from the queue and places them
//such that the queue becomes a Binary Search Tree (BST)
HuffmanNode* buildEncodingTree(const Map<int, int>& freqTable) {

    PriorityQueue<HuffmanNode*> pq;

    for (int ch : freqTable.keys()) {
        HuffmanNode* node = new HuffmanNode;
        node->character = ch;
        node->count = freqTable.get(ch);
        node->one = NULL;//left
        node->zero = NULL; //right

        pq.enqueue(node, freqTable.get(ch));
    }

    buildEncodingTreeHelper(pq);
    return pq.peek();
}

//helper function for turning the priority queue into a BST
void buildEncodingTreeHelper(PriorityQueue<HuffmanNode*>& pq) {

    while (pq.size() > 1) {//do this until there is only one root node

        int priority1 = pq.peekPriority();
        HuffmanNode* leftNode = pq.dequeue();
        int priority2 = pq.peekPriority();
        HuffmanNode* rightNode = pq.dequeue();
        HuffmanNode* newParent = new HuffmanNode;

        newParent->count = priority1 + priority2;
        newParent->character = NOT_A_CHAR;
        newParent->zero = leftNode;
        newParent->one = rightNode;

        pq.enqueue(newParent, newParent->count);
        buildEncodingTreeHelper(pq);
    }
}

//builds an encoding map based on the BST by assiging binary digits
//to all nodes of the tree
Map<int, string> buildEncodingMap(HuffmanNode* encodingTree) {

    Map<int, string> encodingMap;
    buildEncodingMapHelper(encodingTree, encodingMap, "" );
    return encodingMap;
}

//helper function that uses recursion to build the encoding map
void buildEncodingMapHelper(HuffmanNode* encodingTree, Map<int, string>& encodingMap, string binary) {

    if (encodingTree != NULL) {
        if (encodingTree->character != NOT_A_CHAR) {
            encodingMap.add(encodingTree->character, binary);
        }
        buildEncodingMapHelper(encodingTree->one, encodingMap, binary + "1");
        buildEncodingMapHelper(encodingTree->zero, encodingMap, binary + "0");
    }
}

//function that encodes a file using a version of the Huffman Compression Algorithm
void encodeData(istream& input, const Map<int, string>& encodingMap, obitstream& output) {

    int ch = input.get();

    while (ch != EOF) {
        string binary = encodingMap.get(ch);
        writeBitsToOutput(binary, output);
        ch = input.get();
    }

    string binary = encodingMap.get(PSEUDO_EOF);
    writeBitsToOutput(binary, output);
}

//helper function that writes bits to the ouput file
void writeBitsToOutput(string binary, obitstream& output ) {

    for (int i = 0; i < binary.length(); i++) {
        if (binary[i] == '0') {
            output.writeBit(0);
        } else if (binary[i] == '1') {
            output.writeBit(1);
        }
    }
}


void decodeData(ibitstream& input, HuffmanNode* encodingTree, ostream& output) {
    decodeDataHelper(input,encodingTree, encodingTree, output, "" );
}

//helper function that uses recursion to decode a file consisting of binary representations of characters
void decodeDataHelper(ibitstream& input,HuffmanNode* root,HuffmanNode* encodingTree, ostream& output, string binary) {

    if (encodingTree->character == PSEUDO_EOF) {//base case: we reached end of file
        return;
    }

    if (encodingTree->character != NOT_A_CHAR) {
        output.put(encodingTree->character);
        binary = "";
        decodeDataHelper(input, root, root, output, binary);

    } else {
        int bit = input.readBit();
        if (bit == -1) {
            return;
        }

        if (bit == 1) {
            decodeDataHelper(input, root, encodingTree->one, output, binary += "1");//go right
        } else if (bit == 0){
            decodeDataHelper(input, root, encodingTree->zero, output, binary += "0");//go left
        }
    }
}

//uses all of the above functions to compress a file
void compress(istream& input, obitstream& output) {
    Map<int, int> freqTable = buildFrequencyTable(input);
    HuffmanNode* node = buildEncodingTree(freqTable);
    Map<int, string> encodingMap = buildEncodingMap(node);
    rewindStream(input);
    output << freqTable;
    encodeData(input, encodingMap, output);
    freeTree(node);

}

//decompresses a file using the map that is written to the header of the compressed file
void decompress(ibitstream& input, ostream& output) {

        Map<int, int> freqTable;
        input >> freqTable;
        HuffmanNode* root = buildEncodingTree(freqTable);
        decodeData (input,root,output);\
        freeTree(root);
}

//frees memory for each node in the tree
void freeTree(HuffmanNode* node) {

    if(node!=NULL)
    {
        freeTree(node->zero);
        freeTree(node->one);
        free(node);
    }
}


