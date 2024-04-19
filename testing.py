import socket
import json
import threading
import time
from flask import Flask, render_template_string, request, jsonify

app = Flask(__name__)

isRunning = True
game_state = {}

# Function to receive game state from the server
def receive_game_state():
    global isRunning, game_state
    while isRunning:
        try:
            data = client_socket.recv(4096).decode()
            if not data:
                break
            game_state = json.loads(data)
            print(game_state)
            time.sleep(1)
        except Exception as e:
            print("Failed to receive game state:", e)
            break

@app.route('/')
def index():
    return render_template_string('''
        <html>
        <head>
            <title>Blackjack Game</title>
            <script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
            <script>
                $(document).ready(function(){
                    function updateGameState(){
                        $.getJSON("/game_state", function(data){
                            $("#game_state").text(JSON.stringify(data));
                        });
                    }
                    setInterval(updateGameState, 1000);
                });
            </script>
        </head>
        <body>
            <h1>Blackjack Game</h1>
            <p>Game State: <span id="game_state"></span></p>
            <form action="/action" method="post">
                <label for="user_action">Enter action (HIT/STAND/DONE):</label>
                <input type="text" id="user_action" name="user_action">
                <input type="submit" value="Submit">
            </form>
        </body>
        </html>
    ''')

@app.route('/game_state')
def get_game_state():
    global game_state
    return jsonify(game_state)

# Function to send user action to the server
def send_action():
    global isRunning
    while isRunning:
        try:
            bet_message = {
                "type": "BET",
                "betAmount": bet_amount
            }
            client_socket.sendall(json.dumps(bet_message).encode())
            user_action = input("Enter action (HIT/STAND/DONE): ").upper()
            if user_action == "DONE":
                client_socket.sendall(json.dumps({"type": "DONE"}).encode())
                print("Exiting the game.")
                #isRunning = False
                break
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

@app.route('/action', methods=['POST'])
def handle_action():
    user_action = request.form['user_action'].upper()
    if user_action == "DONE":
        client_socket.sendall(json.dumps({"type": "DONE"}).encode())
        print("Exiting the game.")
        return "Exiting the game."
    action_message = {
        "type": "TURN",
        "action": user_action
    }
    client_socket.sendall(json.dumps(action_message).encode())
    return "Action sent: {}".format(user_action)

server_ip = '127.0.0.1'
server_port = 1500

try:
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((server_ip, server_port))
    print("Connected to the server.")

    receive_thread = threading.Thread(target=receive_game_state)
    send_thread = threading.Thread(target=send_action)
    receive_thread.start()

    # Run Flask app in the main thread
    app.run(debug=True)

except KeyboardInterrupt:
    print("Closing the connection.")
    client_socket.close()
finally:
    client_socket.close()