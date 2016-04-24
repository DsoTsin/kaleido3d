import sys

def file2bytes(src_file, variable='', dest_file=''):
	with open(src_file, 'r') as source:
		bytes = source.read()
		print len(bytes)
		if variable=='':
			variable = 'style_css'
		code = 'char %s[] = \n{%s\n};'
		bytescode = ''
		for index in range(0, len(bytes)):
			if index==len(bytes):
				break
			if bytes[index]=='\\':
				bytescode += '\'\\\\\','
			elif bytes[index]=='\n':
				bytescode += '\'\\n\','
			elif bytes[index]=='\'':
				bytescode += '\'\\\'\','
			elif bytes[index]=='\t':
				bytescode += '\'\\t\','
			else:
				bytescode += '\'' + bytes[index] + '\','
			if index%20==0:
				bytescode += '\n\t'
		code = code % (variable, bytescode)
		if dest_file=='':
			dest_file = src_file+'.h'
		dest = open(dest_file, 'w')
		dest.write(code)
		dest.close()
		source.close()


file2bytes(sys.argv[1])