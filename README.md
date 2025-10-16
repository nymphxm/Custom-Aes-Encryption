# Custom-Aes-Encryption

This project is designed to be a lightweight c/c++ header file to encrypt bytes safely and securely. It does not rely on any external packages or libraries besides for things like vector, c string, c standard int, and the memory header.

## How to use:

1. Download the header file and paste it into your existing project.
2. Include your header file in the source file you want to use the encryption class in.
3. Call the "encrypt" or "decrypt" function from the header by doing g_aes->encrypt();
4. For the arguments of the call, first pass in the data you want to encrypt thats in the form of a const std::vector<uint8_t>. Then, for the second argument, pass in your encryption key thats defined as a const std::vector<uint8_t>;
5. Enjoy safe, secure encryption to keep your data safe.
