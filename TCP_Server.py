import socket


def receive_messages(host, port):
    # Create a socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Bind the socket to a specific address and port
    server_socket.bind((host, port))

    # Enable the socket to listen for connections (maximum number of connections: 1)
    server_socket.listen(1)

    print(f"Server is listening on {host}:{port}...")

    # Wait for connections
    client_socket, client_address = server_socket.accept()
    print(f"Connection established from {client_address}!")

    # Receive messages and print them to the console
    while True:
        data = client_socket.recv(1024)
        if not data:
            break
        print(f"Received message: {data.decode('utf-8')}")

    # Close the connection
    client_socket.close()
    server_socket.close()


if __name__ == "__main__":
    HOST = '0.0.0.0'  # Change this to your desired host address
    PORT = 12345  # Change this to your desired port

    while True:
        receive_messages(HOST, PORT)
