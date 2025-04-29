#include <stdio.h>
#include <string.h>

#define LINE_LEN 1024
#define MAX_BUFFER_LEN 5120
#define ENCODE_OUTPUT_PATH "encoding.txt"
#define DECODE_OUTPUT_PATH "decoding.txt"
#define MAX_FILE_PATH 256

#define RED "\033[1;31m"
#define RESET "\033[0m"

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

    // only used for encoding to calculate padding
    int readCount;          // in bytes
    int writeCount;         // in bytes

    // MAX_BUFFER_LEN chunk of the result is stored in resultBuf
    byte resultBuf[MAX_BUFFER_LEN];
    int resultBufIndex;

    // write resultBuf to output file if exceed MAX_BUFFER_LEN
    int exceedBuf;
    byte *outputBufP;
    FILE *outputFileP;
};

void base64Init(struct Base64Ctx *ctx, char output[MAX_BUFFER_LEN], char *outputPath) {
    ctx->value = 0;
    ctx->valueIndex = 0;
    ctx->readCount = 0;
    ctx->writeCount = 0;

    ctx->resultBuf[0] = '\0';
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
                    // don't have to care about adding a '\0' since fwrite can just specify bytes to write
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

    if (ctx->valueIndex != 0) {
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
        // write buffer to output file if buffer max size is reached
        if (ctx->resultBufIndex == MAX_BUFFER_LEN) {
            ctx->exceedBuf = 1;
            fwrite(ctx->resultBuf, 1, ctx->resultBufIndex, ctx->outputFileP);

            // clears buffer
            ctx->resultBuf[0] = '\0';
            ctx->resultBufIndex = 0;
        }

        ctx->resultBuf[ctx->resultBufIndex] = '=';
        ctx->resultBufIndex++;
        ctx->writeCount += 1;
    }

    // write everything left in buffer to file
    fwrite(ctx->resultBuf, 1, ctx->resultBufIndex, ctx->outputFileP);
    
    // write result to output buffer
    if (ctx->exceedBuf == 0) {
        ctx->resultBuf[ctx->resultBufIndex] = '\0';
        strcpy(ctx->outputBufP, ctx->resultBuf);
    }
    
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

    // write result to output buffer
    if (ctx->exceedBuf == 0) {
        ctx->resultBuf[ctx->resultBufIndex] = '\0';
        strcpy(ctx->outputBufP, ctx->resultBuf);
    }

    fclose(ctx->outputFileP);
}

void encode(FILE *fileP, char *msg, char *outputPath) {
    if (fileP != NULL) {
        byte fBuffer[LINE_LEN];
        struct Base64Ctx ctx;
        char output[MAX_BUFFER_LEN];

        // initialize base64 context
        base64Init(&ctx, output, outputPath);
        
        int readLen = 0;
        do {
            // reading binary file
            readLen = fread(fBuffer, 1, sizeof(fBuffer), fileP);
            base64EncodeUpdate(&ctx, fBuffer, readLen);
        } while (readLen > 0);
        
        base64EncodeFinal(&ctx);
    } 
    else if (msg != NULL) {
        struct Base64Ctx ctx;
        char output[MAX_BUFFER_LEN];
        
        // initialize base64 context
        base64Init(&ctx, output, outputPath);

        base64EncodeUpdate(&ctx, msg, strlen(msg));
        
        base64EncodeFinal(&ctx);

        if (ctx.exceedBuf == 0) {
            printf("%s\n", output);
        }
        else {
            printf("Encoding result too large to print to console. Check output file for the result.\n");
        }
    }
}

void decode(FILE *fileP, char *msg, char *outputPath) {
    if (fileP != NULL) { 
        byte fBuffer[LINE_LEN];
        struct Base64Ctx ctx;
        char output[MAX_BUFFER_LEN];
        
        // initialize base64 context
        base64Init(&ctx, output, outputPath);
        
        // reading text file
        while(fgets(fBuffer, LINE_LEN-1, fileP)) {
            base64DecodeUpdate(&ctx, fBuffer, strlen(fBuffer));
        }
    
        base64DecodeFinal(&ctx);
    }
    else if (msg != NULL) {
        struct Base64Ctx ctx;
        char output[MAX_BUFFER_LEN];

        // initialize base64 context
        base64Init(&ctx, output, outputPath);

        base64DecodeUpdate(&ctx, msg, strlen(msg));
    
        base64DecodeFinal(&ctx);
        
        if (ctx.exceedBuf == 0) {
            printf("%s\n", output);
        }
        else {
            printf("Decoding result too large to print to console. Check output file for the result.\n");
        }
    }
}

void printHelp() {
    printf("****************************************\n");
    printf("*                                      *\n");
    printf("*        Base64 Encoder Decoder        *\n");
    printf("*                                      *\n");
    printf("****************************************\n");
    printf("\n");
    printf("./base64 [-e|-d]  [-b FILEPATH]|[TEXT]\n");
    printf("   Encoder transform binary data to a sequence of printable 64 unique characters.\n");
    printf("   Decoder transform Base64 encoding back into binary data.\n");
    printf("   Example (Encoding): ./base64 -e -b example/image01.jpg\n");
    printf("   Example (Decoding): ./base64 -o=decode.jpg -d -b encoding.txt\n");
    printf("\n");
    printf("   Options:\n");
    printf("    -h, --help\t\t\t  Prints help information.\n");
    printf("    -e, --encode\t\t  Performs encoding operation.\n");
    printf("    -d, --decode\t\t  Performs decoding operation.\n");
    printf("    -o, --output-file=FILEPATH\t  Set output file path.\n");
    printf("    -b, --binary\t\t  Set binary mode. Last argument given should be a path\n");
    printf("                \t\t  to the file that you want to perform operation on.\n");
    printf("\n");
    printf("Result will always be automatically written to an output file.\n");
    printf("If -b, --binary option is set, the file binary content at FILEPATH will be encoded.\n");
    printf("Otherwise, if -b, --binary option is not set, then the TEXT will be encoded and the\n");
    printf("result will be printed if buffer is not exceeded.\n");
}

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        printf("No argument given. Use -h or --help for more information.\n");

    } 
    else if (argc == 2) {
        // if only 1 argument is given
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            printHelp();
        }
        else if (!(strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "--encode") == 0) &&
                !(strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--decode") == 0) && 
                !(strcmp(argv[1], "-b") == 0 || strcmp(argv[1], "--binary") == 0) && 
                !(strstr(argv[1], "-o=") != NULL || strstr(argv[1], "--output-file=") != NULL)) {
            
            printf(RED);
            printf("Unrecognized command-line option %s. Use -h or --help for more information.\n", argv[1]);
            printf(RESET);
            return 0;
        }
    }
    else {
        // more than 1 argument
        int helpFlag = 0;
        int encodeFlag = 0;
        int decodeFlag = 0;
        int binaryFlag = 0;
        char *outputPath = NULL;
        char *subject = argv[argc - 1];             // text or filepath depending on binaryFlag
       
        // check if all options given are valid
        for (int i = 1; i < argc - 1; i++) {
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                helpFlag = 1;
            }
            else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--encode") == 0) {
                encodeFlag = 1;
            }
            else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--decode") == 0) {
                decodeFlag = 1;
            }
            else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--binary") == 0) {
                binaryFlag = 1;
            }
            else if (strstr(argv[i], "-o=") != NULL || strstr(argv[i], "--output-file=") != NULL) {
                // extract file path using = delimiter
                strtok(argv[i], "=");
                outputPath = strtok(NULL, "=");

                if (outputPath == NULL) {
                    printf(RED);
                    printf("Output file path is undefined. Default output path will be used.\n");
                    printf(RESET);
                }
            }
            else {
                printf(RED);
                printf("Unrecognized command-line option %s. Use -h or --help for more information.\n", argv[i]);
                printf(RESET);
                return 0;
            }
        }
        
        // check for help flag
        if (helpFlag) {
            printHelp();
            return 0;
        }

        // check for decode or encode flag
        if (encodeFlag && decodeFlag) {
            printf(RED);
            printf("Both operation flag detected, only one can be active at a time.\n");
            printf("Unable to proceed. Try again.\n");
            printf(RESET);
            return 0;
        } 
        else if (!encodeFlag && !decodeFlag) {
            printf(RED);
            printf("No operation flag detected.\n");
            printf("Unable to proceed. Try again.\n");
            printf(RESET);
            return 0;
        }
        
        // do operation
        if (encodeFlag) {            
            // set output path
            outputPath = (outputPath == NULL) ? ENCODE_OUTPUT_PATH : outputPath;

            if (binaryFlag) {
                // read file
                FILE *fileP = fopen(subject, "rb");
                if (fileP == NULL) {
                    printf(RED);
                    printf("File failed to open.\n");
                    printf(RESET);
                    return 1;
                }

                // encoding operation
                printf("Encoding...\n");
                encode(fileP, NULL, outputPath);
                printf("Encoding completed.\n");
            }
            else {
                // encoding operation
                printf("Encoding...\n");
                encode(NULL, subject, outputPath);
                printf("Encoding completed.\n");
            }
        }
        else if (decodeFlag) {
            // set output path
            outputPath = (outputPath == NULL) ? DECODE_OUTPUT_PATH : outputPath;

            if (binaryFlag) {
                // read file
                FILE *fileP = fopen(subject, "rb");
                if (fileP == NULL) {
                    printf(RED);
                    printf("File failed to open.\n");
                    printf(RESET);
                    return 1;
                }

                // decoding operation
                printf("Decoding...\n");
                decode(fileP, NULL, outputPath);
                printf("Decoding completed.\n");
            }
            else {
                // decoding operation
                printf("Decoding...\n");
                decode(NULL, subject, outputPath);
                printf("Decoding completed.\n");
            }
        }
    }

    // TODO: if no printable chars found during decoding such as when
    // 1 base64 char (6 bits) is not able to be fully decoded into 1 byte.
    // TODO: Increase output buffer size.
    return 0;
}