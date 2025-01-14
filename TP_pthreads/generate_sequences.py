import random

length_X = 40000
length_Y = 40000
def generate_sequence(length):
    return ''.join(random.choice('ATCG') for _ in range(length))
with open("X.txt", "w") as file_X:
    file_X.write(generate_sequence(length_X))
with open("Y.txt", "w") as file_Y:
    file_Y.write(generate_sequence(length_Y))

print("Les fichiers X.txt et Y.txt ont été générés avec succès.")