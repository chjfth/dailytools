import os, sys, time
import requests

RANGE_START = 1
RANGE_MAX = 20

# Base URL for the files
base_url = "https://cfvod.kaltura.com/hls/p/2503451/sp/0/serveFlavor/entryId/1_2ctpww6v/v/11/ev/7/flavorId/1_jwmhznmn/name/a.mp4/" # video and audio
#base_url ="https://cfvod.kaltura.com/hls/p/2503451/sp/0/serveFlavor/entryId/1_2ctpww6v/v/21/ev/7/flavorId/1_ly4l9p05/name/a.mp4/" # audio only
filename_ptn = "seg-%d-v1-a1.ts"

print("Base URL: " + base_url)
print("filename pattern: " + filename_ptn)

for i in range(RANGE_START, RANGE_MAX+1):
    
    filename = filename_ptn%(i)
    
    if os.path.exists(filename):
    	continue
    
    file_url = base_url + filename

    while True:
	    try:
	        # Send a GET request to download the file
	        response = requests.get(file_url)
	        response.raise_for_status()  # Check for HTTP errors

	        # Save the file to the local directory
	        with open(filename, 'wb') as file:
	            file.write(response.content)
	        
	        print(f"Downloaded: " + filename)
	        break
	    
	    except requests.exceptions.RequestException as e:
	        print(f"Error an retrying: " + filename)
	        time.sleep(2)

print("Done %s to %s ."%( 
	filename_ptn%(RANGE_START),
	filename_ptn%(RANGE_MAX)
	))
