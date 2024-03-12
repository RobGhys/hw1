#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <vector>

using namespace std;

struct SixteenBits {
    bitset<8> firstByte;
    bitset<8> secondByte;
};

struct X8086Instruction {
    string operation; // mov
    /*
     * Determination bit
     * 0 -> reg register is the source
     * 1 -> reg register is the destination
     */
    int dBit{};
    /*
     * Wide instruction bit
     * 0 -> mov will copy 8 bits
     * 1 -> mov will copy 16 bits
     */
    int wBit{};
};

X8086Instruction &getOperation(const SixteenBits &inputBits, X8086Instruction &result) {
    bitset<6> movBits(string("100010"));
    cout << "movBits: " << movBits << endl;
    size_t firstSixBitsSize = 6;
    bool sameBits = true;
    for (int i = 0; i < firstSixBitsSize; ++i)
    {
        // i+2 because movBits is bitset<6> whereas inputBits.firstByte is bitset<8>
        if (movBits[i] != inputBits.firstByte[i+2]) {
            cout << "false! at i ==" << i << "movBits: " << movBits[i] << " | firstByte: " << inputBits.firstByte[i] << endl;
            sameBits = false;
            break;
        }
    }
    if (sameBits)
    {
        result.operation = "mov";
    }
    return result;
}

X8086Instruction decodeInstruction(SixteenBits inputBits)
{
    X8086Instruction result;

    result = getOperation(inputBits, result);

    result.dBit = inputBits.firstByte[6];
    result.wBit = inputBits.firstByte[7];

    return result;
}

int readBinFile(const string &listing37AssembledPath, bool littleEndian) {
    ifstream inputFile(listing37AssembledPath, ios::binary);
    if (!inputFile)
    {
        cerr << "Could not open file." << endl;
        return 1;
    }

    vector<SixteenBits> sixteenBitsVector;
    uint16_t twoBytes;

    while (inputFile.read(reinterpret_cast<char*>(&twoBytes), sizeof(twoBytes)))
    {
        auto binaryTwoBytes = bitset<16>(twoBytes);
        cout << binaryTwoBytes << endl;
        cout << "***" << endl;

        SixteenBits sixteenBits;
        for (int i = 0; i < 8; ++i)
        {
            if (littleEndian)
            {
                sixteenBits.firstByte[i] = binaryTwoBytes[i];
                sixteenBits.secondByte[i] = binaryTwoBytes[i+8];
            } else {
                cout << "Big endian" << endl;
                sixteenBits.firstByte[i] = binaryTwoBytes[i+8];
                sixteenBits.secondByte[i] = binaryTwoBytes[i];
            }
        }
        sixteenBitsVector.push_back(sixteenBits);
    }
    size_t nb_elem = sixteenBitsVector.size();
    for (int i = 0; i < nb_elem; ++i)
    {
        cout << "Element # " << i + 1 << endl;
        cout << "First Byte: " << sixteenBitsVector[i].firstByte << " | ";
        cout << "Second Byte: " << sixteenBitsVector[i].secondByte << endl;

        X8086Instruction instruction = decodeInstruction(sixteenBitsVector[i]);
        cout << "--- Instruction ---" << endl;
        cout << "Op " << instruction.operation << " | D " << instruction.dBit << " | W " << instruction.wBit << endl;
    }

    inputFile.close();

    return 0;
}

int main()
{
    bool littleEndian = true;
    string listing37AssembledPath = "/home/rob/Documents/Github/computer_enhance/hw1/listing_0037_single_register_mov";
    //string listing38AssembledPath = "/home/rob/Documents/Github/computer_enhance/hw1/listing_0038_many_register_mov";

    cout << "listing37 output: " << endl;
    if(readBinFile(listing37AssembledPath, littleEndian) == 1) { return 1;}
/*    cout << "next...\n" << endl;
    if(readBinFile(listing38AssembledPath, littleEndian) == 1) { return 1;}
    cout << "done." << endl;*/
}