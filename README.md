# base64-encoder-decoder
Base64 encoder transform binary data to a sequence of printable characters, limited to a set of 64 unique characters. The source binary data is taken 6 bits at a time, then this group of 6 bits is mapped to one of 64 unique characters. The decoder transforms the Base64 encoding back into their original binary data.

---
Compilation on Linux (Debian-based distros):

    $ gcc -o base64 base64.c

Run it with the command:

    $ ./base64 <options>... <-b FILEPATH>|<TEXT>

Options:
- `-h`, `--help`    Prints help information.
- `-e`, `--encode`    Performs encoding operation.
- `-d`, `--decode`    Performs decoding operation.
- `-o`, `--output-file=OUTPUTPATH`    Set output file path.
- `-b`, `--binary`   Set binary mode to perform operation on file. Last argument given should 
be a path to the file that you want to perform operation on.

---
### Reflections
When I was doing the Beginner's Guide to the picoGym on picoCTF, I came across a question that requires me to decode a base64 string to get the flag. And that was when I thought that it might be interesting to try to build my own base64 encoder and decoder. I did use online base64 encoders before but I did not really give it that much thought on how it actually works. I know that it isn't too difficult and with my current experience, I feel that I should be able to manage it. <br>
First and foremost, I did some research on [Wikipedia](https://en.wikipedia.org/wiki/Base64) to see how the encoding and decoding works. From there, I learn that the key is to group every 6 bits, convert them to numbers, and lastly convert them into their corresponding Base64 character based on the Base64 table. Every 3 bytes are transformed into 4 Base64 characters. <br>
Decoding is the opposite. The Base64 characters are first converted back to their actual values based on the Base64 table. The first 6 bits of every values are then put together into groups of 8 which represents the original byte. Every 4 Base64 characters are transformed into 3 bytes. <br>
Armed with this knowledge, I went and start building the program. Since this is actually my first time working with data on a bit by bit level, I encountered quite a few of issues throughout my journey. Some of the challenges I faced includes not knowing how to capture certain bits in a byte, reading and writing strings from and to files when I should be reading and writing binary instead, and debugging issues regarding writing the wrong number of bits to the output file leading to wrong decoding results. Some of the issues were solved with a little bit of Googling and others were solved using the debugger or by staring long enough at the code to realize my mistake. From this project, I learned a lot about Base64 and encoding in general as well as bit manipulation. I also got more practice working with struct and command-line arguments.

---
### Attributions
Photo used in example folder is by Lexi Laginess on [Unsplash](https://unsplash.com/photos/a-lamp-post-in-a-garden-nt6N7BS3rPk?utm_content=creditCopyText&utm_medium=referral&utm_source=unsplash)
      