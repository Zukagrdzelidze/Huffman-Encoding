/**********************************************************
 * File: HuffmanEncoding.cpp
 *
 * Implementation of the functions from HuffmanEncoding.h.
 * Most (if not all) of the code that you write for this
 * assignment will go into this file.
 */

#include "HuffmanEncoding.h"

void initializePriorityQueue(PriorityQueue<Node*> &pq, Map<ext_char, int>& frequencies);
void unitTwo(PriorityQueue<Node*> &pq);
void initializeCode(Node* encodingTree, Map<ext_char, Queue<bool> > &code, Queue<bool> &q);

/* Function: getFrequencyTable
 * Usage: Map<ext_char, int> freq = getFrequencyTable(file);
 * --------------------------------------------------------
 * Given an input stream containing text, calculates the
 * frequencies of each character within that text and stores
 * the result as a Map from ext_chars to the number of times
 * that the character appears.
 *
 * This function will also set the frequency of the PSEUDO_EOF
 * character to be 1, which ensures that any future encoding
 * tree built from these frequencies will have an encoding for
 * the PSEUDO_EOF character.
 */
Map<ext_char, int> getFrequencyTable(istream& file) {
	Map<ext_char, int> freqTable;
	char c;
	ext_char ch;
	while(file.get(c)){
		ch = c;
		freqTable[ch]++;
	}
	freqTable.put(PSEUDO_EOF,1);
	return freqTable;	
}

/* Function: buildEncodingTree
 * Usage: Node* tree = buildEncodingTree(frequency);
 * --------------------------------------------------------
 * Given a map from extended characters to frequencies,
 * constructs a Huffman encoding tree from those frequencies
 * and returns a pointer to the root.
 *
 * This function can assume that there is always at least one
 * entry in the map, since the PSEUDO_EOF character will always
 * be	 present.
 */
Node* buildEncodingTree(Map<ext_char, int>& frequencies) {
	PriorityQueue<Node*> pq;
	initializePriorityQueue(pq, frequencies);
	while(pq.size() > 1) unitTwo(pq);
	return pq.dequeue();
}

void initializePriorityQueue(PriorityQueue<Node*> &pq, Map<ext_char, int>& frequencies){
	foreach(ext_char ch in frequencies){
		Node* cell  = new Node;
		cell->weight = frequencies[ch];
		cell->character = ch;
		cell->one = cell->zero = NULL;
		pq.enqueue(cell,cell->weight);
	}
}

void unitTwo(PriorityQueue<Node*> &pq){
	Node* n1 = pq.dequeue();
	Node* n2 = pq.dequeue();
	Node* cell = new Node;
	cell->character = NOT_A_CHAR;
	cell->zero = n1;
	cell->one = n2;
	cell->weight = n1->weight + n2->weight;
	pq.enqueue(cell , cell->weight);
}

/* Function: freeTree
 * Usage: freeTree(encodingTree);
 * --------------------------------------------------------
 * Deallocates all memory allocated for a given encoding
 * tree.
 */
void freeTree(Node* root) {
	if(root->zero != NULL) {
		freeTree(root->zero);
		root->zero = NULL;
	}
	if(root->one != NULL) {
		freeTree(root->one);
		root->one = NULL;
	}
	delete root;
}

/* Function: encodeFile
 * Usage: encodeFile(source, encodingTree, output);
 * --------------------------------------------------------
 * Encodes the given file using the encoding specified by the
 * given encoding tree, then writes the result one bit at a
 * time to the specified output file.
 *
 * This function can assume the following:
 *
 *   - The encoding tree was constructed from the given file,
 *     so every character appears somewhere in the encoding
 *     tree.
 *
 *   - The output file already has the encoding table written
 *     to it, and the file cursor is at the end of the file.
 *     This means that you should just start writing the bits
 *     without seeking the file anywhere.
 */ 
void encodeFile(istream& infile, Node* encodingTree, obstream& outfile) {
	Map<ext_char, Queue<bool> > code;
	Queue<bool> q;
	initializeCode(encodingTree, code, q);
	ext_char ch;
	char c;
	while(infile.get(c)){
		ch = c;
		q = code.get(ch);
		while(!q.isEmpty()) outfile.writeBit(q.dequeue());
	}
	q = code.get(PSEUDO_EOF);
	while(!q.isEmpty()) outfile.writeBit(q.dequeue());
}

void initializeCode(Node* cell, Map<ext_char, Queue<bool> > &code, Queue<bool> &q){
	if(cell == NULL) return;
	if(cell->character != NOT_A_CHAR){
		code.put(cell->character, q);
		return;
	}
	Queue<bool> tmp = q;
	tmp.enqueue(false);
	initializeCode(cell->zero, code, tmp);
	tmp = q;
	tmp.enqueue(true);
	initializeCode(cell->one, code, tmp);
}

/* Function: decodeFile
 * Usage: decodeFile(encodedFile, encodingTree, resultFile);
 * --------------------------------------------------------
 * Decodes a file that has previously been encoded using the
 * encodeFile function.  You can assume the following:
 *
 *   - The encoding table has already been read from the input
 *     file, and the encoding tree parameter was constructed from
 *     this encoding table.
 *
 *   - The output file is open and ready for writing.
 */
void decodeFile(ibstream& infile, Node* encodingTree, ostream& file) {
	Node* cell = encodingTree;
	char ch;
	while(true){
		if(cell->character != NOT_A_CHAR){
			if(cell->character == PSEUDO_EOF) break;
			ch = cell->character;
			file.put(ch);		
			cell = encodingTree;
		}
		if(infile.readBit()) cell = cell->one;
		else cell = cell->zero;
	}
}

/* Function: writeFileHeader
 * Usage: writeFileHeader(output, frequencies);
 * --------------------------------------------------------
 * Writes a table to the front of the specified output file
 * that contains information about the frequencies of all of
 * the letters in the input text.  This information can then
 * be used to decompress input files once they've been
 * compressed.
 *
 * This function is provided for you.  You are free to modify
 * it if you see fit, but if you do you must also update the
 * readFileHeader function defined below this one so that it
 * can properly read the data back.
 */
void writeFileHeader(obstream& outfile, Map<ext_char, int>& frequencies) {
	/* The format we will use is the following:
	 *
	 * First number: Total number of characters whose frequency is being
	 *               encoded.
	 * An appropriate number of pairs of the form [char][frequency][space],
	 * encoding the number of occurrences.
	 *
	 * No information about PSEUDO_EOF is written, since the frequency is
	 * always 1.
	 */
	 
	/* Verify that we have PSEUDO_EOF somewhere in this mapping. */
	if (!frequencies.containsKey(PSEUDO_EOF)) {
		error("No PSEUDO_EOF defined.");
	}
	
	/* Write how many encodings we're going to have.  Note the space after
	 * this number to ensure that we can read it back correctly.
	 */
	outfile << frequencies.size() - 1 << ' ';
	
	/* Now, write the letter/frequency pairs. */
	foreach (ext_char ch in frequencies) {
		/* Skip PSEUDO_EOF if we see it. */
		if (ch == PSEUDO_EOF) continue;
		
		/* Write out the letter and its frequency. */
		outfile << char(ch) << frequencies[ch] << ' ';
	}
}

/* Function: readFileHeader
 * Usage: Map<ext_char, int> freq = writeFileHeader(input);
 * --------------------------------------------------------
 * Reads a table to the front of the specified input file
 * that contains information about the frequencies of all of
 * the letters in the input text.  This information can then
 * be used to reconstruct the encoding tree for that file.
 *
 * This function is provided for you.  You are free to modify
 * it if you see fit, but if you do you must also update the
 * writeFileHeader function defined before this one so that it
 * can properly write the data.
 */
Map<ext_char, int> readFileHeader(ibstream& infile) {
	/* This function inverts the mapping we wrote out in the
	 * writeFileHeader function before.  If you make any
	 * changes to that function, be sure to change this one
	 * too!
	 */
	Map<ext_char, int> result;
	
	/* Read how many values we're going to read in. */
	int numValues;
	infile >> numValues;
	
	/* Skip trailing whitespace. */
	infile.get();
	
	/* Read those values in. */
	for (int i = 0; i < numValues; i++) {
		/* Get the character we're going to read. */
		ext_char ch = infile.get();
		
		/* Get the frequency. */
		int frequency;
		infile >> frequency;
		
		/* Skip the space character. */
		infile.get();
		
		/* Add this to the encoding table. */
		result[ch] = frequency;
	}
	
	/* Add in 1 for PSEUDO_EOF. */
	result[PSEUDO_EOF] = 1;
	return result;
}

/* Function: compress
 * Usage: compress(infile, outfile);
 * --------------------------------------------------------
 * Main entry point for the Huffman compressor.  Compresses
 * the file whose contents are specified by the input
 * ibstream, then writes the result to outfile.  Your final
 * task in this assignment will be to combine all of the
 * previous functions together to implement this function,
 * which should not require much logic of its own and should
 * primarily be glue code.
 */
void compress(ibstream& infile, obstream& outfile) {
	Map<ext_char, int> frequencies = getFrequencyTable(infile);
	infile.rewind();
	writeFileHeader(outfile,frequencies);
	Node* root = buildEncodingTree(frequencies);
	encodeFile(infile, root, outfile);
	freeTree(root);
}

/* Function: decompress
 * Usage: decompress(infile, outfile);
 * --------------------------------------------------------
 * Main entry point for the Huffman decompressor.
 * Decompresses the file whose contents are specified by the
 * input ibstream, then writes the decompressed version of
 * the file to the stream specified by outfile.  Your final
 * task in this assignment will be to combine all of the
 * previous functions together to implement this function,
 * which should not require much logic of its own and should
 * primarily be glue code.
 */
void decompress(ibstream& infile, ostream& outfile) {
	Node* root = buildEncodingTree(readFileHeader(infile));
	decodeFile(infile, root, outfile);
	freeTree(root);
}