#!/usr/bin/env bash

# Should be run from the root of the project
# Generates badge.svg and test-results.svg files in /wiki/ based on test results

# Build the compiler
make

testsPassed=0
testsFailed=0
testsSkipped=0

# Run the tests
testResults=$(make test)

# Print test output for debug
echo "$testResults"

# Parse the test results
while read -r line; do
	if [[ $line == "Tests Passed:"* ]]; then
		testsPassed=$(echo "$line" | awk '{print $3}')
	elif [[ $line == "Tests Failed:"* ]]; then
		testsFailed=$(echo "$line" | awk '{print $3}')
	elif [[ $line == "Tests Untested:"* ]]; then
		testsSkipped=$(echo "$line" | awk '{print $3}')
	fi
done <<< "$testResults"

totalTests=$((testsPassed + testsFailed + testsSkipped))

if [[ $totalTests -eq 0 ]]; then
	# Probably the build failed
	echo "No tests were run. Please check the build output."
	exit
fi

# Percentage passed with one decimal place
percentagePassed=$(echo "scale=1; $testsPassed * 100 / $totalTests" | bc)

# Percentage passed (rounded to the nearest integer)
percentagePassedRounded=$((testsPassed * 100 / totalTests))

# Generate the SVG badge

# Calculate the color of the badge based on the percentage of tests passed
color="#4c1" # Green for 100%, red for 0%
# Automatically convert the percentage to some shade between those two colors
color=$(printf "#%02x%02x%02x" $((255 * (100 - percentagePassedRounded) / 100)) $((255 * percentagePassedRounded / 100)) 0)

# Calculate "progress-bar" width
progressBarWidth=$((percentagePassedRounded * 3))

cat > wiki/badge.svg <<SVG
<svg xmlns="http://www.w3.org/2000/svg" width="300" height="60">
	<linearGradient id="b" x2="0" y2="100%">
		<stop offset="0" stop-color="#bbb" stop-opacity=".1"/>
		<stop offset="1" stop-opacity=".1"/>
	</linearGradient>
	<mask id="a">
		<rect width="300" height="60" rx="3" fill="#fff"/>
	</mask>
	<g mask="url(#a)">
		<path fill="#555" d="M0 0h85v60H0z"/>
		<path fill="#555" d="M85 0h215v60H85z"/>
		<path fill="url(#b)" d="M0 0h300v60H0z"/>
		<rect width="$progressBarWidth" height="60" fill="$color"/>
	</g>
	<g fill="#fff" text-anchor="middle" font-family="DejaVu Sans,Verdana,Geneva,sans-serif" font-size="32">
		<text x="150" y="40" fill="#010101" fill-opacity=".3" alignment-baseline="middle">$percentagePassed% passing</text>
		<text x="150" y="37" alignment-baseline="middle">$percentagePassed% passing</text>
	</g>
</svg>

SVG

# Generate the SVG "Test Results" block

cat > wiki/test-results.svg <<SVG
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!-- Created with Inkscape (http://www.inkscape.org/) -->

<svg
   width="210mm"
   height="122mm"
   viewBox="0 0 210 122"
   version="1.1"
   id="svg5"
   inkscape:version="1.2.2 (b0a8486541, 2022-12-01)"
   sodipodi:docname="test-results.svg"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:svg="http://www.w3.org/2000/svg">
  <sodipodi:namedview
     id="namedview7"
     pagecolor="#ffffff"
     bordercolor="#000000"
     borderopacity="0.25"
     inkscape:showpageshadow="2"
     inkscape:pageopacity="0.0"
     inkscape:pagecheckerboard="0"
     inkscape:deskcolor="#d1d1d1"
     inkscape:document-units="mm"
     showgrid="false"
     inkscape:zoom="0.73851712"
     inkscape:cx="396.74097"
     inkscape:cy="529.43932"
     inkscape:window-width="1920"
     inkscape:window-height="1007"
     inkscape:window-x="0"
     inkscape:window-y="0"
     inkscape:window-maximized="1"
     inkscape:current-layer="layer1" />
  <defs
     id="defs2" />
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <rect
       style="fill:#1c1c1c;stroke-width:0.30677"
       id="rect450"
       width="215"
       height="125"
       x="-1"
       y="-1" />
    <text
       xml:space="preserve"
       style="font-size:19.7556px;fill:#000000;stroke-width:0.264583"
       x="52.869061"
       y="16.114214"
       id="text616"><tspan
         sodipodi:role="line"
         id="tspan614"
         style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:19.7556px;font-family:Roboto;-inkscape-font-specification:Roboto;fill:#f2f2f2;stroke-width:0.264583"
         x="52.869061"
         y="16.114214">Test Results</tspan></text>
    <text
       xml:space="preserve"
       style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:9.87778px;font-family:Roboto;-inkscape-font-specification:Roboto;fill:#f2f2f2;stroke-width:0.264583"
       x="0.75884324"
       y="35.509621"
       id="text772"><tspan
         sodipodi:role="line"
         id="tspan770"
         style="font-size:9.87778px;stroke-width:0.264583"
         x="0.75884324"
         y="35.509621">Total number of tests</tspan></text>
    <text
       xml:space="preserve"
       style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:9.87777px;font-family:Roboto;-inkscape-font-specification:Roboto;fill:#f2f2f2;stroke-width:0.264583"
       x="0.75884348"
       y="52.118938"
       id="text776"><tspan
         sodipodi:role="line"
         id="tspan774"
         style="stroke-width:0.264583"
         x="0.75884348"
         y="52.118938">Tests passing</tspan></text>
    <text
       xml:space="preserve"
       style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:9.87777px;font-family:Roboto;-inkscape-font-specification:Roboto;fill:#f2f2f2;stroke-width:0.264583"
       x="0.75884348"
       y="69.509613"
       id="text780"><tspan
         sodipodi:role="line"
         id="tspan778"
         style="stroke-width:0.264583"
         x="0.75884348"
         y="69.509613">Tests failing</tspan></text>
    <text
       xml:space="preserve"
       style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:9.87777px;font-family:Roboto;-inkscape-font-specification:Roboto;fill:#f2f2f2;stroke-width:0.264583"
       x="0.75884348"
       y="86.408325"
       id="text784"><tspan
         sodipodi:role="line"
         style="stroke-width:0.264583"
         x="0.75884348"
         y="86.408325"
         id="tspan786">Tests skipped</tspan></text>
    <text
       xml:space="preserve"
       style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:19.7556px;font-family:Roboto;-inkscape-font-specification:Roboto;fill:#f2f2f2;stroke-width:0.264583"
       x="31.353237"
       y="116.49284"
       id="text792"><tspan
         sodipodi:role="line"
         id="tspan790"
         style="font-size:19.7556px;stroke-width:0.264583"
         x="31.353237"
         y="116.49284">$percentagePassed% Pass Rate</tspan></text>
    <text
       xml:space="preserve"
       style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:9.87778px;font-family:Roboto;-inkscape-font-specification:Roboto;fill:#f2f2f2;stroke-width:0.264583"
       x="183.18489"
       y="35.022484"
       id="text796"><tspan
         sodipodi:role="line"
         id="tspan794"
         style="font-size:9.87778px;stroke-width:0.264583"
         x="183.18489"
         y="35.022484">$totalTests</tspan></text>
    <text
       xml:space="preserve"
       style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:9.87777px;font-family:Roboto;-inkscape-font-specification:Roboto;fill:#f2f2f2;stroke-width:0.264583"
       x="183.18489"
       y="52.022476"
       id="text800"><tspan
         sodipodi:role="line"
         id="tspan798"
         style="fill:#00ff00;stroke-width:0.264583"
         x="183.18489"
         y="52.022476">$testsPassed</tspan></text>
    <text
       xml:space="preserve"
       style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:9.87777px;font-family:Roboto;-inkscape-font-specification:Roboto;fill:#f2f2f2;stroke-width:0.264583"
       x="183.18489"
       y="69.022476"
       id="text804"><tspan
         sodipodi:role="line"
         id="tspan802"
         style="fill:#ff0000;stroke-width:0.264583"
         x="183.18489"
         y="69.022476">$testsFailed</tspan></text>
    <text
       xml:space="preserve"
       style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:9.87777px;font-family:Roboto;-inkscape-font-specification:Roboto;fill:#f2f2f2;stroke-width:0.264583"
       x="183.18489"
       y="86.022476"
       id="text808"><tspan
         sodipodi:role="line"
         id="tspan806"
         style="fill:#ffcc00;stroke-width:0.264583"
         x="183.18489"
         y="86.022476">$testsSkipped</tspan></text>
  </g>
</svg>

SVG