import random

ops = ["add", "sub", "mul"]

def rpush():
  print("push " + str(random.randint(-500000, 500000)))

def outer(rounds):
  print("push " + str(rounds));
  print("outer:")
  rand_math_seq()

  print("push 1")
  print("sub")

  print("pushr -1")
  print("push 0")
  print("jne outer")

def rand_math_seq():
  max = 500000
  rpush()
  for i in range(max):
    rpush()
    choice = random.sample(ops, 1)[0]
    print(choice);
  print("pop");

outer(10000)
