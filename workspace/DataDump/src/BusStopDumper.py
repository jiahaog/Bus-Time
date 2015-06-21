__author__ = 'JiaHao'

import config
import requests
import traceback
import time

NO_OF_BUS_STOPS = 4752
SKIP_INCREMENT = 50
MAX_RETRIES = 5
BASE_URL = 'http://datamall.mytransport.sg/ltaodataservice.svc/BusStopCodeSet?$skip='

REQUEST_HEADERS = {
    'AccountKey': config.API_KEY,
    'UniqueUserId': config.UUID,
    'accept': 'application/json'
}


def main():

    s = requests.session()
    s.headers.update(REQUEST_HEADERS)

    currentSkip = 0

    while currentSkip < NO_OF_BUS_STOPS:


        getBusStopsPage(s, currentSkip)

        currentSkip += SKIP_INCREMENT

    print 'Completed successfully'

def getBusStopsPage(session, skip):
    print 'making request of skip: ' + str(skip)

    url = BASE_URL + str(skip)
    completed = False

    tries = 0

    while not completed:

        try:

            response = session.get(url)
            appendToFile(response.text)
            completed = True

        except (KeyboardInterrupt, SystemExit):
            raise

        except:
            print 'Error, retrying'
            theTraceBack = traceback.format_exc()
            print theTraceBack

            if tries > MAX_RETRIES:

                completed = True
                writeToLog('==========' + url + ' failed with traceback\n' + theTraceBack + '\n')

            else:
                time.sleep(5*60)




def appendToFile(text):
    with open('../out/dump.json', 'a') as myFile:
        myFile.write(text + ',')

def writeToLog(text):
    with open('../out/log.txt', 'a') as myFile:
        myFile.write(text)

if __name__ == '__main__':

    # s = requests.session()
    # s.headers.update(REQUEST_HEADERS)
    #
    # currentSkip = 0
    #
    #
    # getBusStopsPage(s, 0)
    #
    #
    # print 'Completed successfully'

    main()