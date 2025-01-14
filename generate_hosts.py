for i in range(16):
    with open("hosts"+str(i+1),"w") as file:
        file.write("192.168.10.232:"+str(i+1)+"\n")
        file.write("192.168.10.151:"+str(i+1))