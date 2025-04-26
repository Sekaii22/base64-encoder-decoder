#include <stdio.h>
#include <string.h>

/*
    BUILD COMMAND: gcc -fsanitize=address -o base64 base64.c
*/

// base64 (standard) table
char dictionary[64 + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* 
    Calculate the base64 length after encoding the message. 
*/
int getBase64EncodeLen(char *msg) {
    int msgBitLen = strlen(msg) * 8;

    // no of base64 char needed to encode msg
    int intialLen = (msgBitLen + (6 - 1)) / 6;                    // always round up

    // base64 string length must be a multiple of 4
    // since 3 bytes (24 bits) forms 4 base64 chars
    int finalLen = ((intialLen + (4 - 1)) / 4) * 4;               // always round up

    return finalLen;
}

/*
    Calculate the message length after decoding the base64 encoding.
*/
int getBase64DecodeLen(char *encoding) {
    return (strlen(encoding) * 6) / 8; 
}

/*
    Get starting index of the first occurance of subStr in str.
*/
int getIndexFromStr(char *str, char *subStr) {
    char *result = strstr(str, subStr);
    int pos = result - str;

    return pos;
}

/* 
    Encodes a message into a base64 string. Base64 string is stored in output.
*/
void encode(char *msg, char *output) {
    // each three bytes of msg are converted to four base64 encoding
    int msgLen = strlen(msg);
    int msgIndex = 0;
    
    int finalBase64Len = getBase64EncodeLen(msg);
    char base64Str[finalBase64Len + 1];
    int base64Index = 0;

    /*
    //constructing the base64 string
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
        base64Str[base64Index] = dictionary[value];
        base64Index++;
    }
    */

    // temp value store for constructing encoding char
    unsigned int value = 0;
    int valueIndex = 0;

    while (msgIndex < msgLen) {
        unsigned char currMsgChar = msg[msgIndex];

        // each msg char is 8 bits long
        for (int offset = 0; offset < 8; offset++) {
            value = value << 1;
            value = ((currMsgChar >> (7 - offset)) & 1) | value;
            valueIndex++;

            // every 6 bit read will be converted to encoding char
            if (valueIndex == 6) {
                base64Str[base64Index] = dictionary[value];
                base64Index++;

                // reset value
                valueIndex = 0;
                value = 0;
            }
        }
        
        msgIndex++;
    }

    // add padding until final length is reached
    while (base64Index < finalBase64Len) {
        base64Str[base64Index] = '=';
        base64Index++;
    }

    base64Str[base64Index] = '\0';
    strcpy(output, base64Str);
}

/*
    Decodes base64 encoding to a string. Message is stored in output.
*/
void decode(char *encoding, char *output) {
    // four base64 characters are converted back to three bytes
    int encodingLen = strlen(encoding);
    int encodingIndex = 0;

    char msg[getBase64DecodeLen(encoding) + 1];
    int msgIndex = 0;

    // temp value store for constructing msg char
    unsigned int value = 0;
    int valueIndex = 0;

    while (encodingIndex < encodingLen) {
        if (encoding[encodingIndex] == '=') {
            encodingIndex++;
            value = value >> 2;
            valueIndex = valueIndex - 2 >= 0 ? valueIndex - 2 : 0;
            continue;
        }

        // Use table to translate base64 char to their actual value
        char search[] = {encoding[encodingIndex], '\0'};
        int currEncodingCharVal = getIndexFromStr(dictionary, search);     

        // each encoding char is 6 bits long
        for (int offset = 0; offset < 6; offset++) {
            value = value << 1;
            value = ((currEncodingCharVal >> (5 - offset)) & 1) | value;
            valueIndex++;
            
            // every 8 bit read will be converted to msg char
            if (valueIndex == 8) {
                msg[msgIndex] = (char) value;
                msgIndex++;

                // reset value
                valueIndex = 0;
                value = 0;
            }
        }

        encodingIndex++;
    }

    msg[msgIndex] = '\0';
    strcpy(output, msg);
}

int main(int argc, char *arcv) {
    
    char msg[] = "Many hands make light work.";
    char encoding[getBase64EncodeLen(msg) + 1];
    encode(msg, encoding);
    printf("%s\n", encoding);

    // char encoding[] = "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu";
    // char msg[getBase64DecodeLen(encoding) + 1];
    // decode(encoding, msg);
    // printf("%s\n", msg);

    //TODO: Base64 is usually to encode binary data like image not text, so do that.

    // char search[] = {'Q', '\0'};
    // int pos = getIndexFromStr(dictionary, search);
    // printf("%d\n", pos);

    return 0;
}