import random
import sys

ops = ["add", "sub", "mul"]

def rpush():
  print("push " + str(random.randint(-5000, 5000)))

def rand_math_seq(max):
  rpush()
  for i in range(max):
    rpush()
    choice = random.sample(ops, 1)[0]
    print(choice);
  print("pop");

def outer(rounds, inner):
  print("push " + str(rounds));
  print("outer:")
  rand_math_seq(inner)
  print("push 1")
  print("sub")

  print("pushr -1")
  print("push 0")
  print("jne outer")

outer(int(sys.argv[-2]), int(sys.argv[-1]))
