import socket
import json
import threading
import time
isRunning = True
# Function to receive game state from the server
def receive_game_state():
    global isRunning  # Declare isRunning as global
    while isRunning:
        try:
            data = client_socket.recv(4096).decode()
            if not data:
                break
            if "server_termination" in data:
                print("Server has terminated!")
                isRunning = False
                break
            print(data)
            print("Enter action (HIT/STAND)")
                            
        except Exception as e:
            print("Failed to receive game state:", e)
            break


# Function to send user action to the server
def send_action():
    while isRunning:
        try:

            bet_message = {
                "type": "BET",
                "betAmount": bet_amount
            }
            client_socket.sendall(json.dumps(bet_message).encode())
            # Get user input for action (HIT/STAND)
            user_action = input("Enter action (HIT/STAND): ").upper()

            # Send the action to the server
            action_message = {
                "type": "TURN",
                "action": user_action
            }
            client_socket.sendall(json.dumps(action_message).encode())
        except Exception as e:
            print("Failed to send action:", e)
            break

# Function to get bet amount from the user
def get_bet_amount(balance):
    while True:
        try:
            bet_amount = int(input(f"Enter your bet amount (available balance: {balance}): "))
            if bet_amount > balance:
                print("Insufficient balance. Please enter a valid bet amount.")
            else:
                return bet_amount
        except ValueError:
            print("Invalid input. Please enter a valid number.")

# Set up the connection to the server
server_ip = '127.0.0.1'
server_port = 1500

try:
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((server_ip, server_port))
    print("Connected to the server.")

    # Prompt the user for their initial bet amount
    user_balance = 200
    bet_amount = get_bet_amount(user_balance)

    # Create and start threads for receiving game state and taking user input
    receive_thread = threading.Thread(target=receive_game_state)
    send_thread = threading.Thread(target=send_action)
    receive_thread.start()
    send_thread.start()

    # Wait for the threads to finish (this will never happen as the threads run indefinitely)
    receive_thread.join()
    send_thread.join()

except KeyboardInterrupt:
    print("Closing the connection.")
    client_socket.close()
except Exception as e:
    print("An error occurred:", e)
