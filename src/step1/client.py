#   Script créé lors du cours de Network Security 2
#   Création d'un client SSH qui permet le dialogue avec un serveur SSH
#   Connexion async puis sync
#   Author: Nathan Pichon

import paramiko

paramiko.util.log_to_file('ssh.log')

ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.load_system_host_keys()
ssh.connect('127.0.0.1', username="pichon_b", password="toblerone42")
stdin, stdout, stderr = ssh.exec_command("ls -la")
output = stdout.readlines()
for case in output:
    print case,
ssh.close()