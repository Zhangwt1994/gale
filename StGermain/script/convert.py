
import stgDtd
import stgMetaXsd
import string

def dtd2MetaXsd( xml_text ):
	# Parse DTD
	try:
		dtd = stgDtd.readXML( xml_text )
	except:
		print 'Failed to parse as a StGermain DTD'
		raise

	# Convert DTD-dict to XSD-dict
	try:
		xsd = dtdDict2metaXsdDict( dtd )
	except:
		print 'Failed to convert information from a StGermain Meta DTD to a StGermain Meta XSD'
		raise

	# Write XSD
	try:
		doc = stgMetaXsd.createXML( xsd )
		return doc.toprettyxml()
	except:
		print 'Failed to write to StGermain Meta XSD'
		raise


def dtdDict2metaXsdDict( stgDtdDict ):
	meta = {}

	# Info...
	meta["info"] = {}
	meta["info"]["title"] = stgDtdDict["Name"]
	# Often Author has been left blank... use Organisation where this has occured
	try:
		if stgDtdDict["Author"] != '...':
			meta["info"]["creator"] = stgDtdDict["Author"]
		else:
			meta["info"]["creator"] = stgDtdDict["Organisation"]
	except KeyError:
		meta["info"]["creator"] = stgDtdDict["Organisation"]
	meta["info"]["publisher"] = stgDtdDict["Organisation"]
	meta["info"]["rights"] = stgDtdDict["License"]
	meta["info"]["source"] = stgDtdDict["Location"]
	meta["info"]["subject"] = stgDtdDict["Project"]
	meta["info"]["description"] = stgDtdDict["Description"].replace('\\','\\\\')

	# Code...
	meta["code"] = {}
	#meta["code"]["example-documentation"] = 
	try:
		if stgDtdDict["Example"] != '...':
			meta["code"]["example-code"] = stgDtdDict["Example"]
	except KeyError:
		pass
	try:
		if stgDtdDict["Parent"] != '...':
			meta["code"]["inherits"] = stgDtdDict["Parent"]
	except KeyError:
		pass

	# Implements...
	meta["implements"] = {}
	try:
		if stgDtdDict["Reference"] != '...':
			meta["implements"]["reference"] = stgDtdDict["Reference"]
	except KeyError:
		pass
	try:
		if stgDtdDict["Equation"] != '...':
			meta["implements"]["equation"] = stgDtdDict["Equation"].replace('\\','\\\\')
	except KeyError:
		pass

	# Parameters...
	meta["parameters"] = []
	try:
		for param in stgDtdDict["Params"]:
			p = {}
			p["name"] = param["Name"]
			# Convert value...
			if param["Type"].lower().strip() == "double":
				p["type"] = "xsd:double"
			elif param["Type"].lower().strip() == "int" or param["Type"].lower().strip() == "integer":
				p["type"] = "xsd:int"
			elif param["Type"].lower().strip() == "long":
				p["type"] = "xsd:long"
			elif param["Type"].lower().strip() == "char":
				p["type"] = "xsd:byte"
			elif param["Type"].lower().strip() == "bool":
				p["type"] = "xsd:boolean"
			elif param["Type"].lower().strip() == "string":
				p["type"] = "xsd:string"
			elif param["Type"].lower().strip() == "unsignedint" or param["Type"].lower().strip() == "unsigned int":
				p["type"] = "xsd:unsignedInt"
			elif param["Type"].lower().strip() == "unsignedlong" or param["Type"].lower().strip() == "unsigned long":
				p["type"] = "xsd:unsignedLong"
			elif param["Type"].lower().strip() == "float":
				p["type"] = "xsd:float"
			elif param["Type"].lower().strip() == "list":
				p["type"] = "stg:list"
			else:
				raise RuntimeError( 'Unknown parameter type' )
			try:
				p["default"] = param["Default"]
			except KeyError:
				pass
			try:
				p["documentation"] = param["Description"].replace('\\','\\\\')
			except KeyError:
				pass
			meta["parameters"].append( p )
	except KeyError:
		pass

	# Associations...
	meta["associations"] = []
	try:
		for assoc in stgDtdDict["Dependencies"]:
			a = {}
			a["name"] = assoc["Name"]
			a["type"] = assoc["Type"]
			# Take opporunity to enforce a nillability state (even though XML XSD doesn't require it). Assume nillable=false by default.
			try:
				if assoc["Essential"].lower().strip() == "false" or assoc["Essential"].lower().strip() == "f" or assoc["Essential"].lower().strip() == "no":
					a["nillable"] = "true"
				else:
					a["nillable"] = "false"
			except KeyError:
				a["nillable"] = "false"
			try:
				a["documentation"] = assoc["Description"].replace('\\','\\\\')
			except KeyError:
				pass
			meta["associations"].append( a )
	except KeyError:
		pass

	return meta

# The purpose of this function is to provide the C code for the includes needed by the code generated by 
# "metaXsdDict2stgDictionaryCode" and "metaXsdDict2stgStrings"
def metaXsdDict2stgCodeHeader():
	s = ''
	s += '#include <cstring>\n'
	# Purposely refer to internal StGermain headers, as StGermain/StGermain.h would necessarily exist yet if a meta file exists
	# within StGermain itself.
	s += '#include "StGermain/Base/Foundation/Foundation.h"\n'
	s += '#include "StGermain/Base/IO/IO.h"\n'
	s += '#define MAX_CHAR_SIZE 512\n'

	return s

# The purpose of this function is to write C code that creates a Name for the component type name and the xml meta data
def metaXsdDict2stgStrings( xsdDict ):
	s = ''

	# The Name of the component
	s += 'Name ' + safecvar( xsdDict["info"]["title"] ) +  '_Name = "' + safecval( xsdDict["info"]["title"] ) + '";\n'
	s += 'Name ' + safecvar( xsdDict["info"]["title"] ) +  '_GetName() {\n'
	s += '\treturn ' + safecvar( xsdDict["info"]["title"] ) +  '_Name;\n'
	s += '}\n'
	s += '\n'

	# The xml of the meta of the component
	xsdDoc = stgMetaXsd.createXML( xsdDict )
	xsdTxt = xsdDoc.toprettyxml()
	xsdTxt = safecval( xsdTxt )	
	s += 'Name ' + safecvar( xsdDict["info"]["title"] ) +  '_Meta = "' + xsdTxt + '";\n'
	s += 'Name ' + safecvar( xsdDict["info"]["title"] ) +  '_GetMetadata() {\n'
	s += '\treturn ' + safecvar( xsdDict["info"]["title"] ) +  '_Meta;\n'
	s += '}\n'
	# The _Type variant exists because of macro used for ComponentRegister_Add does a stringify on the Component_Type argument
	s += 'Name ' + safecvar( xsdDict["info"]["title"] ) +  '_Type_GetMetadata() {\n'
	s += '\treturn ' + safecvar( xsdDict["info"]["title"] ) +  '_Meta;\n'
	s += '}\n'

	return s
	

# The purpose of this function is to write C code that creates a StGermain Dictionary that is equivalent to the passed in python
# dictionary representing a StGermain meta Xsd
def metaXsdDict2stgDictionaryCode( xsdDict ):
	s = ''
	s += 'Dictionary* ' + safecval( xsdDict["info"]["title"] ) + '_MetaAsDictionary() {\n'
	s += '\tDictionary* meta;\n'
	s += '\tDictionary* info;\n'
	s += '\tDictionary* code;\n'
	s += '\tDictionary* implements;\n'
	s += '\tDictionary* parameters;\n'

	s += '\tchar _xml[MAX_CHAR_SIZE];\n'
	s += '\tchar _title[MAX_CHAR_SIZE];\n'
	s += '\tchar _creator[MAX_CHAR_SIZE];\n'
	s += '\tchar _publisher[MAX_CHAR_SIZE];\n'
	s += '\tchar _rights[MAX_CHAR_SIZE];\n'
	s += '\tchar _source[MAX_CHAR_SIZE];\n'
	s += '\tchar _subject[MAX_CHAR_SIZE];\n'
	s += '\tchar _description[MAX_CHAR_SIZE];\n'
	s += '\tchar _info[MAX_CHAR_SIZE];\n'
	
	s += '\tchar _example_documentation[MAX_CHAR_SIZE];\n'
	s += '\tchar _example_code[MAX_CHAR_SIZE];\n'
	s += '\tchar _inherits[MAX_CHAR_SIZE];\n'
	s += '\tchar _code[MAX_CHAR_SIZE];\n'
	s += '\tchar _reference[MAX_CHAR_SIZE];\n'
	s += '\tchar _equation[MAX_CHAR_SIZE];\n'
	s += '\tchar _implements[MAX_CHAR_SIZE];\n'
	s += '\tchar _name[MAX_CHAR_SIZE];\n'
	s += '\tchar _type[MAX_CHAR_SIZE];\n'
	s += '\tchar _default[MAX_CHAR_SIZE];\n'
	s += '\tchar _parameters[MAX_CHAR_SIZE];\n'
	s += '\tchar _nillable[MAX_CHAR_SIZE];\n'
	s += '\tchar _documentation[MAX_CHAR_SIZE];\n'
	s += '\tchar _associations[MAX_CHAR_SIZE];\n'
	s += '\tchar _parameter_var[MAX_CHAR_SIZE];\n'
	s += '\tchar _association_var[MAX_CHAR_SIZE];\n'

	s += '\tstd::strcpy( _xml, "xml" ); std::strcpy( _title, "title" ); std::strcpy( _creator, "creator" ); std::strcpy( _publisher, "publisher" );\n'
	s += '\tstd::strcpy( _rights, "rights" ); std::strcpy( _source, "source" ); std::strcpy( _subject, "subject" ); std::strcpy( _description, "description" );\n'
	s += '\tstd::strcpy( _info, "info" ); std::strcpy( _example_documentation, "example-documentation" ); std::strcpy( _example_code, "example-code" );\n'
	s += '\tstd::strcpy( _inherits, "inherits" ); std::strcpy( _code, "code" ); std::strcpy( _reference, "reference" ); std::strcpy( _equation, "equation" );\n'
	s += '\tstd::strcpy( _implements, "_implements" ); std::strcpy( _name, "name" ); std::strcpy( _type, "type" ); std::strcpy( _default, "default" );\n'
	s += '\tstd::strcpy( _parameters, "parameters" ); std::strcpy( _nillable, "nillable" ); std::strcpy( _documentation, "documentation" );\n'
	s += '\tstd::strcpy( _associations, "associations" ); std::strcpy( _parameter_var, "" ); std::strcpy( _association_var, "" );\n'

	for param in xsdDict["parameters"]:
		s+= '\tDictionary* ' + safecvar( param["name"] ) + 'Param;\n'
	s += '\tDictionary* associations;\n'
	for assoc in xsdDict["associations"]:
		s+= '\tDictionary* ' + safecvar( assoc["name"] ) + 'Assoc;\n'
	s += '\n'

	s += '\tmeta = Dictionary_New();\n'
	s += '\n'

	# XML ... (requires metaXsdDict2stgStrings( xsdDict ) to have been called first)
	s += '\tDictionary_Add( meta, _xml, Dictionary_Entry_Value_FromString( ' + safecvar( xsdDict["info"]["title"] ) + '_Meta ));\n'
	s += '\n'
	
	# Info...
	s += '\tinfo = Dictionary_New();\n'
	s += '\tDictionary_Add( info, _title, Dictionary_Entry_Value_FromString( "' + safecval( xsdDict["info"]["title"] ) + '" ));\n'
	s += '\tDictionary_Add( info, _creator, Dictionary_Entry_Value_FromString( "' + safecval( xsdDict["info"]["creator"] ) + '" ));\n'
	s += '\tDictionary_Add( info, _publisher, Dictionary_Entry_Value_FromString( "' + safecval( xsdDict["info"]["publisher"] ) + '" ));\n'
	s += '\tDictionary_Add( info, _rights, Dictionary_Entry_Value_FromString( "' + safecval( xsdDict["info"]["rights"] ) + '" ));\n'
	s += '\tDictionary_Add( info, _source, Dictionary_Entry_Value_FromString( "' + safecval( xsdDict["info"]["source"] ) + '" ));\n'
	s += '\tDictionary_Add( info, _subject, Dictionary_Entry_Value_FromString( "' + safecval( xsdDict["info"]["subject"] ) + '" ));\n'
	s += '\tDictionary_Add( info, _description, Dictionary_Entry_Value_FromString( "' + safecval( xsdDict["info"]["description"] ) + '" ));\n'
	s += '\tDictionary_Add( meta, _info, Dictionary_Entry_Value_FromStruct( info ) );\n'
	s += '\n'

	# Code...
	s += '\tcode = Dictionary_New();\n'
	try:
		s += '\tDictionary_Add( code, _example_documentation, Dictionary_Entry_Value_FromString( "' + safecval( xsdDict["code"]["example-documentation"] ) + '" ));\n'
	except KeyError:
		pass
	try:
		s += '\tDictionary_Add( code, _example_code, Dictionary_Entry_Value_FromString( "' + safecval( xsdDict["code"]["example-code"] ) + '" ));\n'
	except KeyError:
		pass
	try:
		s += '\tDictionary_Add( code, _inherits, Dictionary_Entry_Value_FromString( "' + safecval( xsdDict["code"]["inherits"] ) + '" ));\n'
	except KeyError:
		pass
	s += '\tDictionary_Add( meta, _code, Dictionary_Entry_Value_FromStruct( code ) );\n'
	s += '\n'

	# Implements...
	s += '\timplements = Dictionary_New();\n'
	try:
		s += '\tDictionary_Add( implements, _reference, Dictionary_Entry_Value_FromString( "' + safecval( xsdDict["implements"]["reference"] ) + '" ));\n'
	except KeyError:
		pass
	try:
		# BIG ASSUMPTION: equation is in latex
		s += '\tDictionary_Add( implements, _equation, Dictionary_Entry_Value_FromString( "' + safecvalFromLatex( xsdDict["implements"]["equation"] ) + '" ));\n'
	except KeyError:
		pass
	s += '\tDictionary_Add( meta, _implements, Dictionary_Entry_Value_FromStruct( implements ) );\n'
	s += '\n'

	# Parameters...
	s += '\tparameters = Dictionary_New();\n'
    
	parameter_list=[]
	for param in xsdDict["parameters"]:
		parameter_list.append( safecvar( param["name"] ) ) 
                
	p_i = 0
	for param in xsdDict["parameters"]:
		s += '\tstd::strcpy( _parameter_var, ' + "\"" + parameter_list[p_i]  + "\"" + ');' 
		s += '\t' + safecvar( param["name"] ) + 'Param = Dictionary_New();\n'
		s += '\tDictionary_Add( ' + safecvar( param["name"] ) + 'Param, _name, Dictionary_Entry_Value_FromString( "' + safecval( param["name"] ) + '" ));\n'
		s += '\tDictionary_Add( ' + safecvar( param["name"] ) + 'Param, _type, Dictionary_Entry_Value_FromString( "' + safecval( param["type"] ) + '" ));\n'
		try:
			s += '\tDictionary_Add( ' + safecvar( param["name"] ) + 'Param, _default, Dictionary_Entry_Value_FromString( "' + safecval( param["default"] ) + '" ));\n'
		except KeyError:
			pass
		try:
			s += '\tDictionary_Add( ' + safecvar( param["name"] ) + 'Param, _documentation, Dictionary_Entry_Value_FromString( "' + safecval( param["documentation"] ) + '" ));\n'
		except KeyError:
			pass
		s += '\tDictionary_Add( parameters, _parameter_var, Dictionary_Entry_Value_FromStruct( ' + safecvar( param["name"] ) + 'Param ) );\n'
		s += '\n'
		p_i +=1
		s += '\tmemset( _parameter_var, 0, 512 );\n'

	s += '\tDictionary_Add( meta, _parameters, Dictionary_Entry_Value_FromStruct( parameters ) );\n'
	s += '\n'

	association_list = []
	for assoc in xsdDict["associations"]:
		association_list.append( safecvar( assoc["name"] ) )

	a_i = 0
	# Associations...
	s += '\tassociations = Dictionary_New();\n'

	for assoc in xsdDict["associations"]:
		s += '\tstd::strcpy( _association_var, ' + "\"" + association_list[a_i]  + "\"" + ');' 
		s += '\t' + safecvar( assoc["name"] ) + 'Assoc = Dictionary_New();\n'
		s += '\tDictionary_Add( ' + safecvar( assoc["name"] ) + 'Assoc, _name, Dictionary_Entry_Value_FromString( "' + safecval( assoc["name"] ) + '" ));\n'
		s += '\tDictionary_Add( ' + safecvar( assoc["name"] ) + 'Assoc, _type, Dictionary_Entry_Value_FromString( "' + safecval( assoc["type"] ) + '" ));\n'
		try:
			s += '\tDictionary_Add( ' + safecvar( assoc["name"] ) + 'Assoc, _nillable, Dictionary_Entry_Value_FromString( "' + safecval( assoc["nillable"] ) + '" ));\n'
		except KeyError:
			pass
		try:
			s += '\tDictionary_Add( ' + safecvar( assoc["name"] ) + 'Assoc, _documentation, Dictionary_Entry_Value_FromString( "' + safecval( assoc["documentation"] ) + '" ));\n'
		except KeyError:
			pass
		s += '\tDictionary_Add( associations, _association_var , Dictionary_Entry_Value_FromStruct( ' + safecvar( assoc["name"] ) + 'Assoc ) );\n'
		s += '\n'
		a_i += 1
		s += '\tmemset( _association_var, 0, 512 );\n'

	s += '\tDictionary_Add( meta, _associations, Dictionary_Entry_Value_FromStruct( associations ) );\n'
	s += '\n'
	s += '\treturn meta;\n'
	s += '}\n'

	# The _Type variant exists because of macro used for ComponentRegister_Add does a stringify on the Component_Type argument
	s += 'Dictionary* ' + safecval( xsdDict["info"]["title"] ) + '_Type_MetaAsDictionary() {\n'
	s += '\treturn ' + safecvar( xsdDict["info"]["title"] ) +  '_MetaAsDictionary();\n'
	s += '}\n'

	# Reset char[] variables
	return s


# Convert the python unicode string 's' into an ascii string that is safe to put in a C string as a value
def safecval( s ):
	return s.replace( "\t", "\\t" ).replace( "\n", "\\n" ).replace( "\"", "\\\"" ).encode( 'ascii', 'replace' )


# Convert the python unicode string 's' into as ascii string that is safe to use as a C variable
def safecvar( s ):
	# The followign doesn't work because we use "_" as quasi namespacing for component names:)	
	#r = ''
	#for a in s.encode( 'ascii', 'replace' ):
	#	if a.isalnum():
	#		r += a
	#return r
	return s.replace( " ", "" ).replace( "-", "" ).replace( "\t", "" ).replace( "\n", "" ).replace( "\"", "" ).replace( "&", "" ).replace( "[", "" ).replace( "]", "" ).replace( "<", "" ).replace( ">", "" ).encode( 'ascii', 'ignore' )

# Convert the python unicode string 's' that is of latex into an ascii string that is safe to put in a C string as a value
# TODO This is not working as expected. we are still getting warnings.
def safecvalFromLatex( s ):
	return safecval( s ).replace( "\\", "\\\\" )



