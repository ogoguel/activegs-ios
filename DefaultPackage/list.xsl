<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/list">
<h1>Title</h1>
<xsl:for-each select="config">
	<li>
		<xsl:value-of select="desc"/>
	</li>
</xsl:for-each>
</xsl:template>
</xsl:stylesheet>