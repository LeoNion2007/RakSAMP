import re
import socket
import threading
import time

class ConsoleClient:
    def __init__(self):
        self.nickname = ''
        self.server_ip = ''
        self.server_port = 0
        self.connected = False
        self.sock = None
        self.lock = threading.Lock()

    def input_nickname(self):
        pattern = re.compile(r'^[A-Za-z]+_[A-Za-z]+$')
        while True:
            name = input('Enter nickname (Name_Surname): ')
            if pattern.match(name):
                self.nickname = name
                break
            print('Invalid format. Try again.')

    def input_server(self):
        while True:
            addr = input('Enter server address (ip:port): ')
            if ':' in addr:
                ip, port = addr.split(':', 1)
                try:
                    port = int(port)
                    socket.inet_aton(ip)
                    self.server_ip = ip
                    self.server_port = port
                    break
                except (ValueError, OSError):
                    pass
            print('Invalid address. Try again.')

    def connect(self):
        with self.lock:
            if self.connected:
                print('Already connected.')
                return
            try:
                self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.sock.settimeout(5)
                self.sock.connect((self.server_ip, self.server_port))
                self.connected = True
                print('Connected to {}:{}.'.format(self.server_ip, self.server_port))
            except Exception as e:
                print('Failed to connect:', e)
                self.connected = False

    def disconnect(self):
        with self.lock:
            if self.sock:
                try:
                    self.sock.close()
                except Exception:
                    pass
            self.connected = False
            self.sock = None
            print('Disconnected.')

    def send_chat(self):
        if not self.connected:
            print('Not connected.')
            return
        while True:
            msg = input('Chat (0 to return): ')
            if msg == '0':
                break
            try:
                self.sock.sendall(msg.encode('utf-8'))
                print('Message sent.')
            except Exception as e:
                print('Failed to send:', e)
                self.disconnect()
                break

    def status_printer(self):
        while True:
            with self.lock:
                status = 'Connected' if self.connected else 'Disconnected'
            print('\n[{}]\n'.format(status))
            time.sleep(1)

    def run(self):
        self.input_nickname()
        self.input_server()
        threading.Thread(target=self.status_printer, daemon=True).start()
        while True:
            print('\n1. Connect')
            print('2. Disconnect')
            print('3. Chat')
            print('4. Change nickname')
            print('5. Change server')
            print('6. Quit')
            choice = input('Select option: ')
            if choice == '1':
                self.connect()
            elif choice == '2':
                self.disconnect()
            elif choice == '3':
                self.send_chat()
            elif choice == '4':
                self.input_nickname()
            elif choice == '5':
                self.input_server()
            elif choice == '6':
                self.disconnect()
                break
            else:
                print('Invalid option.')

if __name__ == '__main__':
    client = ConsoleClient()
    client.run()
