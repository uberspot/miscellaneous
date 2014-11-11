#!/usr/bin/python2.7

# Usage: <scriptname> <host> <port:Optional>
# This is a simple example script to interact with pyopenssl in python and check the certificates
#
# Uses PyOpenSSL to:
# Connect to a remote TLS server,
# Print the whole certificate chain including the root certificate
# Send an encrypted HTTP 1.1 GET request for the / page, and
# Print out the server's response (the web page).
# Also verifies that the X.509 certificate provided by the server is valid
#  and belongs to a certificate chain that starts from a root
#  certificate which you trust.
#  in this example mozilla_cacerts.pem is taken from http://curl.haxx.se/ca/cacert.pem

from OpenSSL import SSL
import sys, socket

def verifycallback(conn, cert, errnum, depth, ok):
        """
        This will be called for each certificate in the certificate chain
        by PyOpenSSL with the following arguments: the connection that
        called the method, the certificate in the chain, an error number,
        the depth in the certification chain (0 is root) and a boolean
        (ok) that states if the certificate in the chain is properly
        validated based on the chain or on our root CA certificates or not.
        Also the function prints each certificate in the chain
        """
        global HOST, CERTCOMMONNAME

        if depth == 0 and (errnum == 9 or errnum == 10):
            print("Exiting due to error: Certificate not yet valid or expired")
            sys.exit(1)  # or raise Exception("Certificate not yet valid or expired")

        # Print Cert
        print(("\n===\n"
                 "Certificate Issuer:\n"
                 " - Organization name: {0}\n"
                 " - Organizational unit: {1}\n"
                 "Certificate subject\n"
                 " - Common name: {2}\n"
                 " - Organization name: {3}\n"
                 " - Organization unit: {4}"
                ).format(cert.get_issuer().organizationName,
                         cert.get_issuer().organizationalUnitName,
                         cert.get_subject().commonName,
                         cert.get_subject().organizationName,
                         cert.get_subject().organizationalUnitName) )

        # Verify
        CERTCOMMONNAME = cert.get_subject().commonName
        if errnum == 0:
            if depth != 0:
                return True
            else:
                if CERTCOMMONNAME.endswith(HOST) or HOST.endswith(CERTCOMMONNAME):
                    return True
        else:
            print("Exiting due to error: Certificate used by server is not signed by a root CA we trust!")
            sys.exit(1)

def main():
    argNum = len(sys.argv)

    global HOST, PORT, CERTCOMMONNAME

    # Check arguments provided
    if (argNum < 2):
        print("Exiting due to error: Script needs two arguments to run: <scriptname> <host> <port>")
        sys.exit(1)

    if (argNum == 2):
        print("Port unspecified! Assuming port 443!\n")
    else:
        PORT = int(sys.argv[2])

    HOST = sys.argv[1]

    print("Contacting host: {0} on port {1}\n====================".format(HOST, PORT))

    # Initialized

    ctx = SSL.Context(SSL.TLSv1_METHOD)
    ctx.set_verify(SSL.VERIFY_PEER | SSL.VERIFY_FAIL_IF_NO_PEER_CERT, verifycallback)
    ctx.load_verify_locations("mozilla_cacerts.pem")

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(10)
    sock = SSL.Connection(ctx, s)
    sock.connect((HOST, PORT))
    sock.setblocking(1)

    # send thee http request
    sock.send(("GET / HTTP/1.1\r\n"
                     "Host: {0}\r\n"
                     "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:33.0) Gecko/20100101 Firefox/33.0\r\n"
                     "Connection: close\r\n\r\n").format(HOST))

    # check if server certificate common name matches the provided one
    if CERTCOMMONNAME.endswith(HOST) or HOST.endswith(CERTCOMMONNAME):
        print("\nCertificates Verified!\n")
    else:
        print("\nExiting due to error: Server certificate CommonName does not match provided hostname!\n")
        sys.exit(1)

    # print the http response
    print("HTTP response:")
    while True:
        try:
            buf = sock.recv(4096)
            print(str(buf).split("\r\n"))
        except (SSL.SysCallError, SSL.ZeroReturnError):
            break
        if not buf:
            break

# ----------------

PORT = 443
HOST = ""
CERTCOMMONNAME = ""

if __name__ == "__main__":
    main()
