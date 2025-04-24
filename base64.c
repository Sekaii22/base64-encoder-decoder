#include <stdio.h>
#include <string.h>

// base64 (standard) table
const char dictionary[64 + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* 
    Calculate the final base64 length needed to encode the text data, msg. 
*/
int getBase64Len(char *msg) {
    int msgBitLen = strlen(msg) * 8;

    // no of base64 char needed to encode msg
    int intialLen = (msgBitLen + (6 - 1)) / 6;                    // always round up

    // base64 string length must be a multiple of 4
    // since 3 bytes (24 bits) forms 4 base64 chars
    int finalLen = ((intialLen + (4 - 1)) / 4) * 4;               // always round up

    return finalLen;
}


/* 
    Encodes msg into a base64 string. Base64 string is stored in output.
*/
void encode(char *msg, char *output) {
    int bitIndex = 0;
    int msgBitLen = strlen(msg) * 8;
    int base64Len = getBase64Len(msg);
    int currLen = 0;
    char base64Str[base64Len + 1];

    // constructing the base64 string
    while (bitIndex < msgBitLen) {
        unsigned int value = 0;

        // capture chunk of 6 bits to construct a base64 char
        for (int i = 0; i < 6; i++) {
            int byteIndex = bitIndex / 8;
            int bitOffset = bitIndex % 8;

            // shift left, adds space to append
            value = value << 1;

            if (bitIndex < msgBitLen) {
                // read starting from most significant unread bit (leftmost bit),
                // shift right and get just that 1 bit using & 1
                // append that bit to value using | (or)
                value = ((msg[byteIndex] >> (7 - bitOffset)) & 1) | value;
                bitIndex++;
            }
        }

        // add the base64 char to final string
        base64Str[currLen] = dictionary[value];
        currLen++;
    }

    while (currLen < base64Len) {
        // add padding
        base64Str[currLen] = '=';
        currLen++;
    }

    base64Str[currLen] = '\0';
    strcpy(output, base64Str);
}

int main(int argc, char *arcv) {
    
    char msg[] = "Many hands make light work.";
    char encoding[getBase64Len(msg) + 1];
    encode(msg, encoding);
    printf("%s\n", encoding);

    return 0;
}