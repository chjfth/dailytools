import os, sys, time
import requests

RANGE_START = 1
RANGE_MAX = 890

# Base URL for the files
base_url = "https://cfvod.kaltura.com/hls/p/2503451/sp/0/serveFlavor/entryId/1_2ctpww6v/v/11/ev/7/flavorId/1_jwmhznmn/name/a.mp4/" # video and audio
filename_ptn = "seg-%d-v1-a1.ts"

print("Base URL: " + base_url)
print("filename pattern: " + filename_ptn)


def do_http_downloads(istart, imax):
	
	# We need HTTP keep-alive feature, so use a "session".
	with requests.Session() as session:

		for i in range(istart, imax+1):

			filename = filename_ptn%(i)

			if os.path.exists(filename):
				continue

			file_url = base_url + filename

			try:
				# Send a GET request to download the file
				response = session.get(file_url)
				response.raise_for_status()  # Check for HTTP errors
			except requests.exceptions.RequestException as e:
				print(f"HTTP error and retrying: " + filename)
				raise

			# Save the file to the local directory
			with open(filename, 'wb') as file:
				file.write(response.content)
			
			print(f"Downloaded: " + filename)


if __name__=='__main__':
	
	while True:
		try:
			do_http_downloads(RANGE_START, RANGE_MAX)

		except requests.exceptions.RequestException as e:
			time.sleep(1)
			continue
	
	print("Done %s to %s ."%( 
		filename_ptn%(RANGE_START),
		filename_ptn%(RANGE_MAX)
		))
