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

    // write resultBuf to output file if exceed MAX_BUFFER_LEN,
    // else write to both.
    int exceedBuf;
    byte *outputBufP;
    FILE *outputFileP;
};

void base64Init(struct Base64Ctx *ctx, char output[MAX_BUFFER_LEN], char *outputPath) {
    ctx->value = 0;
    ctx->valueIndex = 0;
    ctx->readCount = 0;
    ctx->writeCount = 0;

    ctx->resultBufIndex = 0;

    ctx->exceedBuf = 0;
    ctx->outputBufP = output;
    ctx->outputFileP = fopen(outputPath, "wb");
}

/* 
    Encodes a message into a base64 string.
*/
void base64EncodeUpdate(struct Base64Ctx *ctx, byte *msg, int msgLen) {
    // each three bytes of msg are converted to four base64 encoding
    int msgIndex = 0;
    ctx->readCount += msgLen;

    while (msgIndex < msgLen) {
        byte currMsgChar = msg[msgIndex];

        // each msg char is 8 bits long
        for (int offset = 0; offset < 8; offset++) {

            // add space
            ctx->value = ctx->value << 1;

            // read 1 bit from currMsgChar to value
            ctx->value = ((currMsgChar >> (7 - offset)) & 1) | ctx->value;
            ctx->valueIndex++;

            // every 6 bit read will be converted to encoding char
            if (ctx->valueIndex == 6) {

                // write buffer to output file if buffer max size is reached
                if (ctx->resultBufIndex == MAX_BUFFER_LEN) {
                    ctx->exceedBuf = 1;
                    // ctx->resultBuf[ctx->resultBufIndex + 1] = '\0';
                    // fputs(ctx->resultBuf, ctx->outputFileP);
                    fwrite(ctx->resultBuf, 1, ctx->resultBufIndex, ctx->outputFileP);

                    // clears buffer
                    ctx->resultBuf[0] = '\0';
                    ctx->resultBufIndex = 0;
                }

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
    int finalBase64Len = (ctx->readCount + (3 - 1)) / 3;        // always round up
    finalBase64Len = finalBase64Len * 4;

    if (ctx->value != 0) {
        int zeroPaddingSize = 6 - ctx->valueIndex;

        // adds zero padding
        for (int i = 0; i < zeroPaddingSize; i++) {
            ctx->value = ctx->value << 1;
        }

        // write buffer to output file if buffer max size is reached
        if (ctx->resultBufIndex == MAX_BUFFER_LEN) {
            ctx->exceedBuf = 1;
            fwrite(ctx->resultBuf, 1, ctx->resultBufIndex, ctx->outputFileP);

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
    
    // write everything left in buffer to file
    // fputs(ctx->resultBuf, ctx->outputFileP);
    fwrite(ctx->resultBuf, 1, ctx->resultBufIndex, ctx->outputFileP);
    fclose(ctx->outputFileP);
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
    Decodes base64 encoding back to binary.
*/
void base64DecodeUpdate(struct Base64Ctx *ctx, char *encoding, int encodingLen) {
    int encodingIndex = 0;

    while (encodingIndex < encodingLen) {
        byte encodingChar = encoding[encodingIndex];

        if (encodingChar == '=') {
            ctx->value = ctx->value >> 2;
            ctx->valueIndex = (ctx->valueIndex - 2 >= 0) ? ctx->valueIndex - 2 : 0;
            encodingIndex++;
            continue;
        }

        // Use table to translate base64 char to their actual numerically assigned value
        char search[] = {encoding[encodingIndex], '\0'};
        int currEncodingCharVal = getIndexFromStr(dictionary, search); 

        // each encoding char is 6 bits long
        for (int offset = 0; offset < 6; offset++) {
            
            // add space
            ctx->value = ctx->value << 1;

            // read 1 bit from currMsgChar to value
            ctx->value = ((currEncodingCharVal >> (5 - offset)) & 1) | ctx->value;
            ctx->valueIndex++;

            // every 8 bit read to value is a decoded byte
            if (ctx->valueIndex == 8) {

                // write buffer to output file if buffer max size is reached
                if (ctx->resultBufIndex == MAX_BUFFER_LEN) {
                    ctx->exceedBuf = 1;
                    fwrite(ctx->resultBuf, 1, ctx->resultBufIndex, ctx->outputFileP);

                    // clears buffer
                    ctx->resultBuf[0] = '\0';
                    ctx->resultBufIndex = 0;
                }


                ctx->resultBuf[ctx->resultBufIndex] = ctx->value;
                ctx->resultBufIndex++;

                // reset value
                ctx->valueIndex = 0;
                ctx->value = 0;
            }
        }
        
        encodingIndex++;
    }
}

void base64DecodeFinal(struct Base64Ctx *ctx) {
    // write everything left in buffer to file as binary using fwrite
    fwrite(ctx->resultBuf, 1, ctx->resultBufIndex, ctx->outputFileP);

    // clears buffer
    // ctx->resultBuf[0] = '\0';
    // ctx->resultBufIndex = 0;

    ctx->resultBuf[ctx->resultBufIndex] = '\0';
    strcpy(ctx->outputBufP, ctx->resultBuf);
    fclose(ctx->outputFileP);
}

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        printf("No argument given.\n");

        // debug
        struct Base64Ctx ctx;
        byte msg[] = "a";
        char encoding[] = "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu";
        byte output[MAX_BUFFER_LEN];

        // base64Init(&ctx, output, OUTPUT_PATH);
        base64Init(&ctx, output, OUTPUT_PATH);

        // base64EncodeUpdate(&ctx, msg, sizeof(msg));
        base64DecodeUpdate(&ctx, encoding, strlen(encoding));

        // base64EncodeFinal(&ctx);
        base64DecodeFinal(&ctx);

        printf("output: %s\n", output);

    } else {
        //printf("%s\n", argv[1]);

        // read file
        FILE *fileP = fopen(argv[1], "rb");
        if (fileP == NULL) {
            printf("File failed to open.\n");
            return 1;
        }
        
        byte fBuffer[LINE_LEN];
        struct Base64Ctx ctx;
        char output[MAX_BUFFER_LEN];

        // initialize base64 context
        base64Init(&ctx, output, OUTPUT_PATH);

        // encoding operation
        // printf("Encoding...\n");
        // int readLen = 0;
        // do {
        //     // reading binary file
        //     readLen = fread(fBuffer, 1, sizeof(fBuffer), fileP);
        //     base64EncodeUpdate(&ctx, fBuffer, readLen);
        // } while (readLen > 0);
        
        // base64EncodeFinal(&ctx);
        
        // decoding operation
        // reading text file
        printf("Decoding...\n");
        while(fgets(fBuffer, LINE_LEN-1, fileP)) {
            base64DecodeUpdate(&ctx, fBuffer, strlen(fBuffer));
        }

        base64DecodeFinal(&ctx);
    }

    return 0;
}