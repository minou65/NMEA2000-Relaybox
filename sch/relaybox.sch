<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.6.2">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="1.27" unitdist="mm" unit="mm" style="lines" multiple="1" display="yes" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="88" name="SimResults" color="9" fill="1" visible="yes" active="yes"/>
<layer number="89" name="SimProbes" color="9" fill="1" visible="yes" active="yes"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="frames" urn="urn:adsk.eagle:library:229">
<description>&lt;b&gt;Frames for Sheet and Layout&lt;/b&gt;</description>
<packages>
</packages>
<symbols>
<symbol name="DINA4_L" urn="urn:adsk.eagle:symbol:13867/1" library_version="1">
<frame x1="0" y1="0" x2="264.16" y2="180.34" columns="4" rows="4" layer="94" border-left="no" border-top="no" border-right="no" border-bottom="no"/>
</symbol>
<symbol name="DOCFIELD" urn="urn:adsk.eagle:symbol:13864/1" library_version="1">
<wire x1="0" y1="0" x2="71.12" y2="0" width="0.1016" layer="94"/>
<wire x1="101.6" y1="15.24" x2="87.63" y2="15.24" width="0.1016" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="5.08" width="0.1016" layer="94"/>
<wire x1="0" y1="5.08" x2="71.12" y2="5.08" width="0.1016" layer="94"/>
<wire x1="0" y1="5.08" x2="0" y2="15.24" width="0.1016" layer="94"/>
<wire x1="101.6" y1="15.24" x2="101.6" y2="5.08" width="0.1016" layer="94"/>
<wire x1="71.12" y1="5.08" x2="71.12" y2="0" width="0.1016" layer="94"/>
<wire x1="71.12" y1="5.08" x2="87.63" y2="5.08" width="0.1016" layer="94"/>
<wire x1="71.12" y1="0" x2="101.6" y2="0" width="0.1016" layer="94"/>
<wire x1="87.63" y1="15.24" x2="87.63" y2="5.08" width="0.1016" layer="94"/>
<wire x1="87.63" y1="15.24" x2="0" y2="15.24" width="0.1016" layer="94"/>
<wire x1="87.63" y1="5.08" x2="101.6" y2="5.08" width="0.1016" layer="94"/>
<wire x1="101.6" y1="5.08" x2="101.6" y2="0" width="0.1016" layer="94"/>
<wire x1="0" y1="15.24" x2="0" y2="22.86" width="0.1016" layer="94"/>
<wire x1="101.6" y1="35.56" x2="0" y2="35.56" width="0.1016" layer="94"/>
<wire x1="101.6" y1="35.56" x2="101.6" y2="22.86" width="0.1016" layer="94"/>
<wire x1="0" y1="22.86" x2="101.6" y2="22.86" width="0.1016" layer="94"/>
<wire x1="0" y1="22.86" x2="0" y2="35.56" width="0.1016" layer="94"/>
<wire x1="101.6" y1="22.86" x2="101.6" y2="15.24" width="0.1016" layer="94"/>
<text x="1.27" y="1.27" size="2.54" layer="94">Date:</text>
<text x="12.7" y="1.27" size="2.54" layer="94">&gt;LAST_DATE_TIME</text>
<text x="72.39" y="1.27" size="2.54" layer="94">Sheet:</text>
<text x="86.36" y="1.27" size="2.54" layer="94">&gt;SHEET</text>
<text x="88.9" y="11.43" size="2.54" layer="94">REV:</text>
<text x="1.27" y="19.05" size="2.54" layer="94">TITLE:</text>
<text x="1.27" y="11.43" size="2.54" layer="94">Document Number:</text>
<text x="17.78" y="19.05" size="2.54" layer="94">&gt;DRAWING_NAME</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="DINA4_L" urn="urn:adsk.eagle:component:13919/1" prefix="FRAME" uservalue="yes" library_version="1">
<description>&lt;b&gt;FRAME&lt;/b&gt;&lt;p&gt;
DIN A4, landscape with extra doc field</description>
<gates>
<gate name="G$1" symbol="DINA4_L" x="0" y="0"/>
<gate name="G$2" symbol="DOCFIELD" x="162.56" y="0" addlevel="must"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="FRAME1" library="frames" library_urn="urn:adsk.eagle:library:229" deviceset="DINA4_L" device=""/>
</parts>
<sheets>
<sheet>
<plain>
<wire x1="11.43" y1="148.59" x2="11.43" y2="113.03" width="0.1524" layer="94"/>
<wire x1="11.43" y1="113.03" x2="13.97" y2="113.03" width="0.1524" layer="94"/>
<wire x1="13.97" y1="113.03" x2="29.21" y2="113.03" width="0.1524" layer="94"/>
<wire x1="29.21" y1="113.03" x2="80.01" y2="113.03" width="0.1524" layer="94"/>
<wire x1="80.01" y1="113.03" x2="80.01" y2="148.59" width="0.1524" layer="94"/>
<wire x1="80.01" y1="148.59" x2="11.43" y2="148.59" width="0.1524" layer="94"/>
<wire x1="13.97" y1="113.03" x2="13.97" y2="120.65" width="0.1524" layer="94"/>
<wire x1="13.97" y1="120.65" x2="29.21" y2="120.65" width="0.1524" layer="94"/>
<circle x="16.51" y="116.84" radius="2.032" width="0.1524" layer="94"/>
<circle x="21.59" y="116.84" radius="2.032" width="0.1524" layer="94"/>
<circle x="26.67" y="116.84" radius="2.032" width="0.1524" layer="94"/>
<wire x1="29.21" y1="113.03" x2="29.21" y2="120.65" width="0.1524" layer="94"/>
<wire x1="29.21" y1="120.65" x2="44.45" y2="120.65" width="0.1524" layer="94"/>
<wire x1="44.45" y1="120.65" x2="44.45" y2="113.03" width="0.1524" layer="94"/>
<circle x="31.75" y="116.84" radius="2.032" width="0.1524" layer="94"/>
<circle x="36.83" y="116.84" radius="2.032" width="0.1524" layer="94"/>
<circle x="41.91" y="116.84" radius="2.032" width="0.1524" layer="94"/>
<wire x1="31.75" y1="148.59" x2="31.75" y2="140.97" width="0.1524" layer="94"/>
<wire x1="31.75" y1="140.97" x2="16.51" y2="140.97" width="0.1524" layer="94"/>
<wire x1="16.51" y1="140.97" x2="16.51" y2="148.59" width="0.1524" layer="94"/>
<circle x="29.21" y="144.78" radius="2.032" width="0.1524" layer="94"/>
<circle x="24.13" y="144.78" radius="2.032" width="0.1524" layer="94"/>
<circle x="19.05" y="144.78" radius="2.032" width="0.1524" layer="94"/>
<text x="17.526" y="120.65" size="1.778" layer="94" rot="R90">NO1</text>
<text x="32.512" y="120.65" size="1.778" layer="94" rot="R90">NO2</text>
<text x="22.352" y="120.65" size="1.778" layer="94" rot="R90">COM1</text>
<text x="37.592" y="120.65" size="1.778" layer="94" rot="R90">COM2</text>
<text x="27.432" y="120.65" size="1.778" layer="94" rot="R90">NC1</text>
<text x="42.672" y="120.65" size="1.778" layer="94" rot="R90">NC2</text>
<text x="28.194" y="140.97" size="1.778" layer="94" rot="R270">VCC</text>
<text x="23.368" y="140.97" size="1.778" layer="94" rot="R270">GND</text>
<text x="18.288" y="140.97" size="1.778" layer="94" rot="R270">5V</text>
<circle x="153.67" y="154.94" radius="1.27" width="0.1524" layer="94"/>
<circle x="153.67" y="151.13" radius="1.27" width="0.1524" layer="94"/>
<circle x="153.67" y="147.32" radius="1.27" width="0.1524" layer="94"/>
<circle x="153.67" y="143.51" radius="1.27" width="0.1524" layer="94"/>
<circle x="153.67" y="139.7" radius="1.27" width="0.1524" layer="94"/>
<circle x="153.67" y="135.89" radius="1.27" width="0.1524" layer="94"/>
<circle x="153.67" y="132.08" radius="1.27" width="0.1524" layer="94"/>
<circle x="153.67" y="128.27" radius="1.27" width="0.1524" layer="94"/>
<circle x="153.67" y="124.46" radius="1.27" width="0.1524" layer="94"/>
<circle x="153.67" y="120.65" radius="1.27" width="0.1524" layer="94"/>
<circle x="157.48" y="154.94" radius="1.27" width="0.1524" layer="94"/>
<circle x="157.48" y="151.13" radius="1.27" width="0.1524" layer="94"/>
<circle x="157.48" y="147.32" radius="1.27" width="0.1524" layer="94"/>
<circle x="157.48" y="143.51" radius="1.27" width="0.1524" layer="94"/>
<circle x="157.48" y="139.7" radius="1.27" width="0.1524" layer="94"/>
<circle x="157.48" y="135.89" radius="1.27" width="0.1524" layer="94"/>
<circle x="157.48" y="132.08" radius="1.27" width="0.1524" layer="94"/>
<circle x="157.48" y="128.27" radius="1.27" width="0.1524" layer="94"/>
<circle x="157.48" y="124.46" radius="1.27" width="0.1524" layer="94"/>
<circle x="157.48" y="120.65" radius="1.27" width="0.1524" layer="94"/>
<circle x="200.66" y="154.94" radius="1.27" width="0.1524" layer="94"/>
<circle x="200.66" y="151.13" radius="1.27" width="0.1524" layer="94"/>
<circle x="200.66" y="147.32" radius="1.27" width="0.1524" layer="94"/>
<circle x="200.66" y="143.51" radius="1.27" width="0.1524" layer="94"/>
<circle x="200.66" y="139.7" radius="1.27" width="0.1524" layer="94"/>
<circle x="200.66" y="135.89" radius="1.27" width="0.1524" layer="94"/>
<circle x="200.66" y="132.08" radius="1.27" width="0.1524" layer="94"/>
<circle x="200.66" y="128.27" radius="1.27" width="0.1524" layer="94"/>
<circle x="200.66" y="124.46" radius="1.27" width="0.1524" layer="94"/>
<circle x="200.66" y="120.65" radius="1.27" width="0.1524" layer="94"/>
<circle x="204.47" y="154.94" radius="1.27" width="0.1524" layer="94"/>
<circle x="204.47" y="151.13" radius="1.27" width="0.1524" layer="94"/>
<circle x="204.47" y="147.32" radius="1.27" width="0.1524" layer="94"/>
<circle x="204.47" y="143.51" radius="1.27" width="0.1524" layer="94"/>
<circle x="204.47" y="139.7" radius="1.27" width="0.1524" layer="94"/>
<circle x="204.47" y="135.89" radius="1.27" width="0.1524" layer="94"/>
<circle x="204.47" y="132.08" radius="1.27" width="0.1524" layer="94"/>
<circle x="204.47" y="128.27" radius="1.27" width="0.1524" layer="94"/>
<circle x="204.47" y="124.46" radius="1.27" width="0.1524" layer="94"/>
<circle x="204.47" y="120.65" radius="1.27" width="0.1524" layer="94"/>
<wire x1="151.13" y1="157.48" x2="207.01" y2="157.48" width="0.1524" layer="94"/>
<wire x1="207.01" y1="157.48" x2="207.01" y2="118.11" width="0.1524" layer="94"/>
<wire x1="207.01" y1="118.11" x2="151.13" y2="118.11" width="0.1524" layer="94"/>
<wire x1="151.13" y1="118.11" x2="151.13" y2="157.48" width="0.1524" layer="94"/>
<wire x1="171.45" y1="148.59" x2="171.45" y2="140.97" width="0.1524" layer="94"/>
<wire x1="171.45" y1="140.97" x2="172.72" y2="140.97" width="0.1524" layer="94"/>
<wire x1="172.72" y1="140.97" x2="175.26" y2="140.97" width="0.1524" layer="94"/>
<wire x1="175.26" y1="140.97" x2="177.8" y2="140.97" width="0.1524" layer="94"/>
<wire x1="177.8" y1="140.97" x2="180.34" y2="140.97" width="0.1524" layer="94"/>
<wire x1="180.34" y1="140.97" x2="181.61" y2="140.97" width="0.1524" layer="94"/>
<wire x1="181.61" y1="140.97" x2="181.61" y2="148.59" width="0.1524" layer="94"/>
<wire x1="181.61" y1="148.59" x2="180.34" y2="148.59" width="0.1524" layer="94"/>
<wire x1="180.34" y1="148.59" x2="177.8" y2="148.59" width="0.1524" layer="94"/>
<wire x1="177.8" y1="148.59" x2="175.26" y2="148.59" width="0.1524" layer="94"/>
<wire x1="175.26" y1="148.59" x2="172.72" y2="148.59" width="0.1524" layer="94"/>
<wire x1="172.72" y1="148.59" x2="171.45" y2="148.59" width="0.1524" layer="94"/>
<wire x1="172.72" y1="148.59" x2="172.72" y2="151.13" width="0.1524" layer="94"/>
<wire x1="175.26" y1="148.59" x2="175.26" y2="151.13" width="0.1524" layer="94"/>
<wire x1="177.8" y1="148.59" x2="177.8" y2="151.13" width="0.1524" layer="94"/>
<wire x1="180.34" y1="148.59" x2="180.34" y2="151.13" width="0.1524" layer="94"/>
<wire x1="172.72" y1="140.97" x2="172.72" y2="138.43" width="0.1524" layer="94"/>
<wire x1="175.26" y1="140.97" x2="175.26" y2="138.43" width="0.1524" layer="94"/>
<wire x1="177.8" y1="140.97" x2="177.8" y2="138.43" width="0.1524" layer="94"/>
<wire x1="180.34" y1="140.97" x2="180.34" y2="138.43" width="0.1524" layer="94"/>
<text x="172.72" y="142.24" size="1.016" layer="95">IC1
MCP2562</text>
<text x="151.13" y="115.57" size="1.778" layer="95">Extension board</text>
<text x="53.34" y="124.46" size="1.778" layer="95">ESP32_Relay_X2</text>
</plain>
<instances>
<instance part="FRAME1" gate="G$1" x="0" y="0" smashed="yes"/>
<instance part="FRAME1" gate="G$2" x="162.56" y="0" smashed="yes">
<attribute name="LAST_DATE_TIME" x="175.26" y="1.27" size="2.54" layer="94"/>
<attribute name="SHEET" x="248.92" y="1.27" size="2.54" layer="94"/>
<attribute name="DRAWING_NAME" x="180.34" y="19.05" size="2.54" layer="94"/>
</instance>
</instances>
<busses>
</busses>
<nets>
<net name="N$1" class="0">
<segment>
<wire x1="153.67" y1="135.89" x2="153.67" y2="137.668" width="0.1524" layer="91"/>
<wire x1="153.67" y1="137.668" x2="172.72" y2="137.668" width="0.1524" layer="91"/>
<wire x1="172.72" y1="137.668" x2="172.72" y2="138.43" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$2" class="0">
<segment>
<wire x1="157.48" y1="120.65" x2="175.26" y2="120.65" width="0.1524" layer="91"/>
<wire x1="175.26" y1="120.65" x2="175.26" y2="138.43" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$3" class="0">
<segment>
<wire x1="200.66" y1="154.94" x2="195.58" y2="154.94" width="0.1524" layer="91"/>
<wire x1="195.58" y1="154.94" x2="195.58" y2="120.65" width="0.1524" layer="91"/>
<wire x1="195.58" y1="120.65" x2="177.8" y2="120.65" width="0.1524" layer="91"/>
<wire x1="177.8" y1="120.65" x2="177.8" y2="138.43" width="0.1524" layer="91"/>
<wire x1="180.34" y1="151.13" x2="180.34" y2="154.94" width="0.1524" layer="91"/>
<wire x1="180.34" y1="154.94" x2="195.58" y2="154.94" width="0.1524" layer="91"/>
<junction x="195.58" y="154.94"/>
</segment>
</net>
<net name="N$4" class="0">
<segment>
<wire x1="157.48" y1="143.51" x2="166.37" y2="143.51" width="0.1524" layer="91"/>
<wire x1="166.37" y1="143.51" x2="166.37" y2="130.81" width="0.1524" layer="91"/>
<wire x1="166.37" y1="130.81" x2="180.34" y2="130.81" width="0.1524" layer="91"/>
<wire x1="180.34" y1="130.81" x2="180.34" y2="138.43" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$6" class="0">
<segment>
<wire x1="153.67" y1="154.94" x2="153.67" y2="152.908" width="0.1524" layer="91"/>
<wire x1="153.67" y1="152.908" x2="172.72" y2="152.908" width="0.1524" layer="91"/>
<wire x1="172.72" y1="152.908" x2="172.72" y2="150.876" width="0.1524" layer="91"/>
</segment>
</net>
<net name="CANH(WHITE)" class="0">
<segment>
<wire x1="175.26" y1="151.13" x2="175.26" y2="167.64" width="0.1524" layer="91"/>
<label x="175.26" y="160.02" size="1.778" layer="95" rot="R90"/>
</segment>
</net>
<net name="CANL(BLUE)" class="0">
<segment>
<wire x1="177.8" y1="151.13" x2="177.8" y2="167.64" width="0.1524" layer="91"/>
<label x="177.8" y="160.02" size="1.778" layer="95" rot="R90"/>
</segment>
</net>
<net name="GND(BLACK)" class="0">
<segment>
<wire x1="24.13" y1="144.78" x2="24.13" y2="161.29" width="0.1524" layer="91"/>
<label x="24.13" y="153.67" size="1.778" layer="95" rot="R90"/>
</segment>
</net>
<net name="+12V(RED)" class="0">
<segment>
<wire x1="29.21" y1="144.78" x2="29.21" y2="161.29" width="0.1524" layer="91"/>
<label x="29.21" y="153.67" size="1.778" layer="95" rot="R90"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
<compatibility>
<note version="6.3" minversion="6.2.2" severity="warning">
Since Version 6.2.2 text objects can contain more than one line,
which will not be processed correctly with this version.
</note>
<note version="8.2" severity="warning">
Since Version 8.2, EAGLE supports online libraries. The ids
of those online libraries will not be understood (or retained)
with this version.
</note>
<note version="8.3" severity="warning">
Since Version 8.3, EAGLE supports URNs for individual library
assets (packages, symbols, and devices). The URNs of those assets
will not be understood (or retained) with this version.
</note>
</compatibility>
</eagle>
