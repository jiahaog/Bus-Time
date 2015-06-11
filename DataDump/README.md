# Data Dump

How we get a complete set of Singapore bus stop data

## Data Sources

Data is extracted from **[MyTransport DataMall](http://www.mytransport.sg/content/mytransport/home/dataMall.html)**.

### Dynamic Bus Stops Data

API Endpoint: `http://datamall.mytransport.sg/ltaodataservice.svc/BusStopCodeSet?$skip=`

Contains the description, roadname, and bus stop id, but not the location of the bus stop.

However, data is provided in sets of `50`, depending onn the `$skip=` parameter, so we have to make repeated calls to get all the data.

### Static Geospartial Bus Stop Location

Download File [here](http://www.mytransport.sg/content/dam/mytransport/DataMall_StaticData/Geospatial/BusStops.zip)

Contains the latitude and longitude of the bus stops and the bus stop ID, but some entries lack the description field, and this data source does not contain the road name of the bus stop.

--------------
Hence, we need to combine these two data sources to finally generate a json file of data, with the following fields:

- Road Name
- Bus Stop ID
- Location (latitude & longitude)
- Description

## How it works


1.`BusStopDumper.py` will query DataMall and dump the dynamic data
2. `dumpTransformer.py` will convert the dynamic data into a nicer form
3. Manually download the Static Geospartial Bus Stop Data
4. Transform the downloaded `.zip` data file at http://ogre.adc4gis.com to `.geojson` with parameter`Target SRS: WGS84`. Note: Somehow the latitude and longitude for the locations are reversed, so we have to reverse it back.
5. `transformGeojson` will reduce the `.geojson` file and populate empty fields with the dumped dynamic data, as well as add the road name.

## Result 
The result is a JSON object with a single key `data` which contains an array of all the bus stops as objects.

### Keys

Keys are reduced in length to reduce the size of the dump.

`r`: Road Name
`n`: Bus Stop ID
`l`: Location [latitude, longitude]
`d`: Bus stop description

### Example

```json
{
	"data": [
		{
			"r": "Blk 457",
			"n": "28521",
			"l": [
				1.350983295716035,
				103.72110869761
			],
			"d": "BLK 457"
		},
		{
			//...
		}
	]
}
```

> Written with [StackEdit](https://stackedit.io/).