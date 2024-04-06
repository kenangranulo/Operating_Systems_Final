import socket
import sys

def get_user_action():
    while True:
        action = input("Action to you - start/hit/stand/quit: ").strip().lower()
        commands = ['hit', 'stand', 'quit', 'start']
        if action in commands:
            return action
        else:
            print("Invalid action. 'Please choose start', 'hit', 'stand', or 'quit'")

def main(server_host, server_port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.connect((server_host, server_port))
            print("Connected to server.")
        except ConnectionError as e:
            print(f"Unable to connect to the server: {e}")
            sys.exit(1)

        while True:
            game_state = s.recv(1024).decode('utf-8')
            if not game_state:
                print("Disconnected from server.")
                break
            elif "wins" in game_state or "tie" in game_state:
                print(game_state)  # Display the outcome
            elif '|' in game_state:
                print_game_state(game_state)
            else:
                print(game_state)  # Print non-game messages directly
            action = get_user_action()
            if action == "quit":
                break
            
            s.sendall(action.encode('utf-8'))

if __name__ == "__main__":
    HOST = '127.0.0.1'
    PORT = 3000

    if len(sys.argv) > 2:
        HOST, PORT = sys.argv[1], int(sys.argv[2])

    main(HOST, PORT)
