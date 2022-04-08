#!/usr/bin/python

''' Import Libraries --------------------------------------------------------- '''
import os
import configparser

''' User Settings ------------------------------------------------------------ '''
ini_fname = "platformio.ini"

''' Set Directories ---------------------------------------------------------- '''
working_dir = os.getcwd()
web_dir = working_dir + "/lib/webpage"

''' read INI file ------------------------------------------------------------ '''
config = configparser.ConfigParser()
config.read(ini_fname)

build_flags = []
for s in config.sections():
    try:
        build_flags.append(config[s]["build_flags"].split('\n'))
    except KeyError:
        pass

# flatten list of lists
flat_list = [item for sublist in build_flags for item in sublist]
build_flags = flat_list

# parse build_flags into dictionary
bf_dic = {}
for li in build_flags:
    if(li.find("-D") >= 0):
        key = li.replace("-D", "").split("=")[0].strip()
        value = li.replace("-D", "").split("=")[1].strip()
        bf_dic[key] = value

''' Get all html, js, css files in /webpage/ directory ----------------------- '''
files_to_convert = []
for file in os.listdir(web_dir):
    if (file.endswith(".html") | (file.endswith(".js")) | (file.endswith(".css"))):
        files_to_convert.append(os.path.join(web_dir, file))


''' Write Temp Files --------------------------------------------------------- '''
temp_files_to_convert = []
for f in files_to_convert:
    temp_file = f + ".tmp"
    temp_files_to_convert.append(temp_file)

    with open(temp_file, 'w') as tf, open(f, 'r') as html:
        js = html.read()

        # replace the keys with values
        for k,v in bf_dic.items():
            js = js.replace(k, v)
        
        tf.write(js)

''' Write header files ------------------------------------------------------- '''
print("\nWriting header files...")
for f in temp_files_to_convert:
    fbn = os.path.basename(f).replace(".tmp", "")
    print(fbn)
    
    header_fname = "{}.h".format(fbn)
    header_path = os.path.join(web_dir, header_fname)

    hdefine = "INC_{}_".format(header_fname.replace(".", "_").upper())
    pdefine = "PAGE_{}_{}".format(fbn.split(".")[0].lower(), fbn.split(".")[-1].upper())

    with open(header_path, 'w') as header, open(f, 'r') as html:
        header.write("#ifndef {}\n".format(hdefine))
        header.write("#define {}\n\n".format(hdefine))
        header.write("#include <Arduino.h>\n\n")
        header.write('const char {}[] PROGMEM = R"=====(\n'.format(pdefine))
        header.write(html.read())
        header.write(')=====";\n')
        header.write("#endif")

''' Clean up temp files ------------------------------------------------------ '''
for f in temp_files_to_convert:
    os.remove(f)

print("Done! \n\n")