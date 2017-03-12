import sys,os
value=1024
for i in range (1,11):
    command = './my_client ' + str(i) + ' ' + str(value)
    value=value*2
    print command
    os.system(command)
i=i+1
command = './my_client ' + str(i) + ' ' + str(value-100)
os.system(command)

value=1024
for i in range (1,11):
    command = './my_client_enc ' + str(i) + ' ' + str(value)
    value=value*2
    print command
    os.system(command)
i=i+1
command = './my_client_enc ' + str(i) + ' ' + str(value-100)
os.system(command)


for i in range (1,12):
    command = './my_client_get ' + str(i)
    print command
    os.system(command)

