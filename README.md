# MiD_idxcrypt

A fork of idxcrypt originally created by Mounir IDRASSI.

Small Windows/Linux utility for strong file encryption based on AES-CBC-256 and strong PBKDF2 key derivation.

The AES256 key is derived from password using PBKDF2 with 500000 iterations.

PBKDF2 supports SHA-256, SHA-384, SHA-512. SHA-256 is the default.

CBC mode is used for AES with a randomly generated IV that is unique to each file.

For PBKDF2, a random salt is generated for each file to protect again Rainbow-table attacks. It has a size of 16 bytes when SHA-256 is used and a size of 64 bytes otherwise.

All cryptographic operations are done using ```MiDAesLib_Static```, ```MiDHashLib_Static```, ```MiDHmacLib_Static``` and ```MiD_PBKDF2_Static```, 4 small open-source libraries which provide high-level APIs to perform AES, Hashing, Hmac and PBKDF2-Hmac operations.

These libraries make use of OpenSSL Crypto API, and can be found here : <https://github.com/ElMostafaIdrassi>

-------------------------------------------------------------------------------------------------

Copyright (c) 2017 El Mostafa IDRASSI <mostafa.idrassi@tutanota.com>. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

-------------------------------------------------------------------------------------------------

Copyright OpenSSL 2017
Contents licensed under the terms of the OpenSSL license
See http://www.openssl.org/source/license.html for details

-------------------------------------------------------------------------------------------------

Usage : 

 - To encrypt an entire folder : idxcrypt InputFolder Password OutputFolder [/d] [/hash algo]
 
 - To encrypt a file : idxcrypt InputFile Password OutputFile [/d] [/hash_algo]

If /d is omitted, then an encryption is performed.
If /d is specified, then a decryption is performed.

If /hash is ommited, the SHA-256 is used by PBKDF2.
if /hash is specified, then the hash algorithm indicated by algo parameter is used.
Possible values for algo are: sha256, sha384 and sha512.
