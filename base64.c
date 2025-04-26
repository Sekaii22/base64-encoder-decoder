#include <stdio.h>
#include <string.h>

#define LINE_LEN 1024
#define MAX_BUFFER_LEN 5120
#define OUTPUT_PATH "base64.txt"

typedef unsigned char byte;

/*
    BUILD COMMAND: gcc -fsanitize=address -o base64 base64.c
*/

// base64 (standard) table
char dictionary[64 + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

struct Base64Ctx {
    // temp value store for constructing encoding char
    byte value;
    int valueIndex;
    int readCount;          // in bytes
    int writeCount;         // in bytes

    // MAX_BUFFER_LEN chunk of the result is stored in resultBuf
    byte resultBuf[MAX_BUFFER_LEN];
    int resultBufIndex;

    // write to resultBuf only to output file if exceed MAX_BUFFER_LEN,
    // else write to both.
    int exceedBuf;
    char *outputBufP;
    FILE *outputFileP;
};

void base64EncodeInit(struct Base64Ctx *ctx, char output[MAX_BUFFER_LEN], char *outputPath) {
    ctx->value = 0;
    ctx->valueIndex = 0;
    ctx->readCount = 0;
    ctx->writeCount = 0;

    ctx->resultBufIndex = 0;

    ctx->exceedBuf = 0;
    ctx->outputBufP = output;
    ctx->outputFileP = fopen(outputPath, "w");
}

void base64EncodeUpdate(struct Base64Ctx *ctx, byte *msg, int msgLen) {
    // each three bytes of msg are converted to four base64 encoding
    int msgIndex = 0;
    ctx->readCount += msgLen;

    while (msgIndex < msgLen) {
        byte currMsgChar = msg[msgIndex];

        // each msg char is 8 bits long
        for (int offset = 0; offset < 8; offset++) {
            // write buffer to output file if buffer max size is reached
            if (ctx->resultBufIndex + 1 == MAX_BUFFER_LEN - 1) {
                ctx->exceedBuf = 1;
                ctx->resultBuf[ctx->resultBufIndex + 1] = '\0';
                fputs(ctx->resultBuf, ctx->outputFileP);

                // clears buffer
                ctx->resultBuf[0] = '\0';
                ctx->resultBufIndex = 0;
            }

            // add space
            ctx->value = ctx->value << 1;

            // read 1 bit from currMsgChar to value
            ctx->value = ((currMsgChar >> (7 - offset)) & 1) | ctx->value;
            ctx->valueIndex++;

            // every 6 bit read will be converted to encoding char
            if (ctx->valueIndex == 6) {
                ctx->resultBuf[ctx->resultBufIndex] = dictionary[ctx->value];
                ctx->resultBufIndex++;
                ctx->writeCount += 1;

                // reset value
                ctx->valueIndex = 0;
                ctx->value = 0;
            }
        }
        
        msgIndex++;
    }
}

void base64EncodeFinal(struct Base64Ctx *ctx) {
    // int inputBitCount = ctx->readCount * 8;

    // // no of base64 char needed to encode msg
    // int intialBase64Len = (inputBitCount + (6 - 1)) / 6;                      // always round up

    // // base64 string length must be a multiple of 4
    // // since 3 bytes (24 bits) forms 4 base64 chars
    // int finalBase64Len = ((intialBase64Len + (4 - 1)) / 4) * 4;               // always round up

    int finalBase64Len = (ctx->readCount + (3 - 1)) / 3;        // always round up
    finalBase64Len = finalBase64Len * 4;

    if (ctx->value != 0) {
        int zeroPaddingSize = 6 - ctx->valueIndex;

        // adds zero padding
        for (int i = 0; i < zeroPaddingSize; i++) {
            ctx->value = ctx->value << 1;
        }

        // write buffer to output file if buffer max size is reached
        if (ctx->resultBufIndex + 1 == MAX_BUFFER_LEN - 1) {
            ctx->exceedBuf = 1;
            ctx->resultBuf[ctx->resultBufIndex + 1] = '\0';
            fputs(ctx->resultBuf, ctx->outputFileP);

            // clears buffer
            ctx->resultBuf[0] = '\0';
            ctx->resultBufIndex = 0;
        }

        ctx->resultBuf[ctx->resultBufIndex] = dictionary[ctx->value];
        ctx->resultBufIndex++;
        ctx->writeCount += 1;
    }

    // add padding until final length is reached
    while (ctx->writeCount < finalBase64Len) {
        ctx->resultBuf[ctx->resultBufIndex] = '=';
        ctx->resultBufIndex++;
        ctx->writeCount += 1;
    }

    // write to output file and output buffer
    ctx->resultBuf[ctx->resultBufIndex] = '\0';
    if (ctx->exceedBuf == 0) {
        strcpy(ctx->outputBufP, ctx->resultBuf);
    } else {
        printf("Encoding too large to print to console. Check output file for the encoding result.\n");
    }
    
    fputs(ctx->resultBuf, ctx->outputFileP);
    fclose(ctx->outputFileP);
}


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

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        printf("No argument given.\n");

        // debug
        // char msg[] = "a";
        // char encoding[getBase64EncodeLen(msg) + 1];
        // encode(msg, encoding);
        // printf("%s\n", encoding);
        
        // char encoding[] = "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu";
        // char msg[getBase64DecodeLen(encoding) + 1];
        // decode(encoding, msg);
        // printf("%s\n", msg);

        struct Base64Ctx ctx;
        byte msg[] = "a";
        char output[MAX_BUFFER_LEN];

        base64EncodeInit(&ctx, output, OUTPUT_PATH);

        base64EncodeUpdate(&ctx, msg, sizeof(msg));

        base64EncodeFinal(&ctx);

        printf("output: %s\n", output);

    } else {
        //printf("%s\n", argv[1]);
        FILE *fileP = fopen(argv[1], "rb");
        if (fileP == NULL) {
            printf("File failed to open.\n");
            return 1;
        }
        byte fBuffer[LINE_LEN];

        // FILE *outputP = fopen("base64.txt", "w");
        // char fBuffer[LINE_LEN];
        
        // while(fgets(fBuffer, LINE_LEN - 1, fileP)) {
        //     char encoding[getBase64EncodeLen(fBuffer) + 1];
        //     encode(fBuffer, encoding);
        //     fputs(encoding, outputP);
        // }

        struct Base64Ctx ctx;
        char output[MAX_BUFFER_LEN];

        base64EncodeInit(&ctx, output, OUTPUT_PATH);

        int readLen = 0;
        do {
            readLen = fread(fBuffer, 1, sizeof(fBuffer), fileP);
            base64EncodeUpdate(&ctx, fBuffer, readLen);
        } while (readLen > 0);

        base64EncodeFinal(&ctx);

        printf("output: %s\n", output);
    }


    //TODO: Base64 is usually to encode binary data like image not text, so do that.

    return 0;
}