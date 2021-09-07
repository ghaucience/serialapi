<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:fo="http://www.w3.org/1999/XSL/Format" >
<xsl:output method="text" omit-xml-declaration="yes" indent="no"/>
<xsl:key name="ClassName" match="zw_classes/cmd_class" use="@name" />
<xsl:key name="ClassKey" match="zw_classes/cmd_class" use="@key" />

<xsl:template name="const_validator">
	if(passed >=cmdLen) return PARSE_FAIL;
	switch(cmd[passed]) {
	<xsl:for-each select="./const">
	case <xsl:value-of select="./@flagmask"/>: /* <xsl:value-of select="./@flagname"/> */</xsl:for-each>
		break;
	default:
		return UNKNOWN_PARAMETER;
	}
	passed++;
</xsl:template>

<xsl:template name="enum_validator">
	if(passed >=cmdLen) return PARSE_FAIL;
	switch(cmd[passed]) {
	<xsl:for-each select="./enum">
	case <xsl:value-of select="./@key"/>: /* <xsl:value-of select="./@name"/> */ </xsl:for-each>
		break;
	default:
		return UNKNOWN_PARAMETER;
	}
	passed++;
</xsl:template>



<xsl:template name="bitmask_validator">
	<xsl:choose>
	<xsl:when test="./bitmask/@paramoffs = '255'">
		passed = cmdLength;
	</xsl:when>
	<xsl:otherwise>
	<xsl:for-each select="./bitmask">
	<xsl:call-template name="var_length_validator"/>
	</xsl:for-each>
	</xsl:otherwise>
	</xsl:choose>	

</xsl:template>

<xsl:template name="var_length_validator">
	{
		/*TODO this is not quite right...*/
		int len_offset = <xsl:value-of select="./@paramoffs"/>;
		if(len_offset >= cmdLen) return PARSE_FAIL;
		passed += (cmd[len_offset] &amp; <xsl:value-of select="./@lenmask"/>) >> <xsl:value-of select="./@lenoffs"/>  ;
	}
</xsl:template>

<xsl:template name="array_validator">
	<xsl:choose>
	<xsl:when test="./arrayattrib/@len = '255'">
		<xsl:for-each select="./arrayattrib">
		<xsl:call-template name="var_length_validator"/>
		</xsl:for-each>
	</xsl:when>
	<xsl:otherwise>
	passed +=<xsl:value-of select="./arrayattrib/@len"/>;
	</xsl:otherwise>
	</xsl:choose>	
</xsl:template>


<xsl:template name="variant_validator">
	<xsl:choose>
	<xsl:when test="./variant/@paramoffs = '255'">
	{
		int variant_start = passed; 
		//the length of the variant is determined by the command length
		//....
	}
	</xsl:when>
	<xsl:otherwise>
	<xsl:for-each select="./variant">
	{
		/*TODO this is not quite right...*/
		int len_offset = passed - (<xsl:value-of select="../@key"/> ) +(<xsl:value-of select="./@paramoffs"/> )  ;
                int size_offset = (
                <xsl:if test="./@sizeoffs">
		  <xsl:value-of select="./@sizeoffs"/>
                </xsl:if>
                <xsl:if test="not(./@sizeoffs)">
                  0
                </xsl:if>
                );
		if(len_offset >= cmdLen) return PARSE_FAIL;
		passed += ((cmd[len_offset] &amp; <xsl:value-of select="./@sizemask"/>) >> size_offset) + (0<xsl:value-of select="./@sizechange"/>)  ;
	}
	</xsl:for-each>
	</xsl:otherwise>
	</xsl:choose>
</xsl:template>


<xsl:template name="byte_validator">
	<xsl:if test="./@hasdefines ='true'">
	if(passed >=cmdLen) return PARSE_FAIL;
	switch (cmd[passed])
	{
		<xsl:for-each select="./bitflag">
		case <xsl:value-of select="./@key"/>: /*  <xsl:value-of select="./@flagname"/>
		</xsl:for-each>	
		case default:
			return UNKNOWN_PARAMETER;
	}	
	</xsl:if>
	passed++;
</xsl:template>


<xsl:template name="multi_array_validator">
	{
	if(passed >=cmdLen) return PARSE_FAIL;
	<xsl:variable name="select_key" select="concat('0x0', ./multi_array/paramdescloc/@param)"/>
	<xsl:variable name="select_node" select="../param[@key=$select_key]"/>
	/* Selecting on <xsl:value-of select="$select_node/@name"/> */
	if( <xsl:value-of select="$select_node/@key"/> >=cmdLen ) return PARSE_FAIL;
	switch( cmd[<xsl:value-of select="$select_node/@key"/>] )
	{
	<xsl:for-each select="./multi_array">
	case <xsl:value-of select="count(preceding-sibling::multi_array)"/>:
		switch( cmd[passed] )
		{	
		<xsl:for-each select="./bitflag">
		case <xsl:value-of select="@flagmask"/>: /*  <xsl:value-of select="@flagname"/>*/</xsl:for-each>
		default:
			return UNKNOWN_PARAMETER;
		}
		break;
	</xsl:for-each>
	default:
		return UNKNOWN_PARAMETER;	
	}	
	
	}
	passed++;
</xsl:template>



<xsl:template name="variant_group_validator">
	/* Variang group  <xsl:value-of select="./@name"/> */
	{
		int variant_length;
	<xsl:choose>
	<xsl:when test="./@paramOffs = '0xFF'">
		variant_length= -1;
	</xsl:when>
	<xsl:otherwise>	
		<!-- xsl:variable name="select_key" select="concat('0x0', ./@paramOffs)"/-->
		<!-- xsl:variable name="select_node" select="preceding-sibling::param[@key=$select_key]"/-->
	
		int length_offset = passed - (<xsl:value-of select="./@key"/> )  + (<xsl:value-of select="./@paramOffs"/> );
		if(length_offset >= cmdLen) return PARSE_FAIL;
		variant_length = (cmd[length_offset]&amp;<xsl:value-of select="./@sizemask"/> ) >> <xsl:value-of select="./@sizeoffs"/>;
	</xsl:otherwise>
	</xsl:choose>

		/* parsing variant group */
		while( (passed &lt; cmdLen) &amp;&amp; (variant_length>0) ) {
		<xsl:for-each select="./param">
		<xsl:call-template name="parameter_validator"/>
		</xsl:for-each>
		variant_length--;
		}
	}
</xsl:template>

<xsl:template name="parameter_validator">
	/* parsing <xsl:value-of select="./@type"/> --- <xsl:value-of select="./@name"/> */
	<xsl:choose>
	<xsl:when test="@type = 'BYTE'"><xsl:call-template name="byte_validator"/></xsl:when>
	<xsl:when test="@type = 'WORD'">passed+=2;</xsl:when>
	<xsl:when test="@type = 'DWORD'">passed+=4;</xsl:when>
	<xsl:when test="@type = 'BIT_24'">passed+=3;</xsl:when>
	<xsl:when test="@type = 'ARRAY'"><xsl:call-template name="array_validator"/> </xsl:when>
	<xsl:when test="@type = 'BITMASK'">passed++;</xsl:when>
	<xsl:when test="@type = 'STRUCT_BYTE'">passed++;</xsl:when>
	<xsl:when test="@type = 'ENUM'"><xsl:call-template name="enum_validator"/> </xsl:when>
	<xsl:when test="@type = 'ENUM_ARRAY'"><xsl:call-template name="enum_validator"/> </xsl:when>
	<xsl:when test="@type = 'MULTI_ARRAY'"><xsl:call-template name="multi_array_validator"/> </xsl:when>
	<xsl:when test="@type = 'CONST'"> <xsl:call-template name="const_validator"/> </xsl:when>
	<xsl:when test="@type = 'VARIANT'"><xsl:call-template name="variant_validator"/></xsl:when>
	</xsl:choose>
</xsl:template>

<xsl:template name="cmd_validator">
static validator_result_t <xsl:value-of select="../@name"/>_v<xsl:value-of select="../@version"/>_<xsl:value-of select="./@name"/>_validator(uint8_t* cmd, int cmdLen ) {
	int passed=0;
	<xsl:variable name="offset" select="0"/>
	<xsl:variable name="param_offset"/>
	
	<xsl:for-each select="./param">
	<xsl:call-template name="parameter_validator"/>
	</xsl:for-each>
	<xsl:for-each select="variant_group">
	<xsl:call-template name="variant_group_validator"/>
	</xsl:for-each>
	
	if(passed >cmdLen) return PARSE_FAIL;

	return PARSE_OK;
}
</xsl:template>


<xsl:template match="/">

<xsl:variable name="cmd_classes" select="zw_classes/cmd_class[generate-id() = generate-id(key('ClassKey', @key)[last()])]"/>
#include "ZW_command_validator.h"
#include "ZW_typedefs.h"

	<xsl:for-each select="zw_classes/cmd_class">
	<xsl:for-each select="./cmd">
	<xsl:call-template name="cmd_validator"/>
	</xsl:for-each>
	</xsl:for-each>
	
	<xsl:for-each select="zw_classes/cmd_class">
static validator_result_t <xsl:value-of select="./@name"/>_v<xsl:value-of select="./@version"/>_validator(uint8_t* cmd, int cmdLen )
{
	switch( cmd[1] ) {
	<xsl:for-each select="./cmd">
	case  <xsl:value-of select="./@key"/>: /* <xsl:value-of select="./@name"/> */
		return <xsl:value-of select="../@name"/>_v<xsl:value-of select="../@version"/>_<xsl:value-of select="./@name"/>_validator(cmd+2,cmdLen-2);</xsl:for-each>
	}
	return UNKNOWN_COMMAND;
}
	</xsl:for-each>


validator_result_t ZW_command_validator(uint8_t* cmd, int cmdLen, int* version  ) {
	if(cmdLen &lt; 2) return PARSE_FAIL;
	validator_result_t result;	
	validator_result_t result_temp;	
	
	switch( cmd[0] ) {	
	<xsl:for-each select="$cmd_classes">	
	case  <xsl:value-of select="./@key"/>:
		<xsl:variable name="class" select="./@key"/>
		<xsl:for-each select="/zw_classes/cmd_class[@key = $class]">
		result_temp = <xsl:value-of select="./@name"/>_v<xsl:value-of select="./@version"/>_validator(cmd,cmdLen) ;
		if(result_temp == PARSE_OK) {
			*version = <xsl:value-of select="./@version"/>;
			result = PARSE_OK;
		}		
		</xsl:for-each>
		/*If we have succseeded  in parsing at least one version return then its ok.*/
		if(result == PARSE_OK) {
			return PARSE_OK;
		}
		return result_temp;
	</xsl:for-each>
	
	}

	return UNKNOWN_CLASS;
}

	</xsl:template>
	
</xsl:stylesheet>
