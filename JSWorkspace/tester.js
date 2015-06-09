
const dateString = '2015-06-09T14:25:49+00:00';
/**
 *
 * @param arrivalString utc date string
 * @returns {string} e.g. '1m 20s'
 */
function getTimeToArrival(arrivalString) {

    const utcArrival = Date.parse(arrivalString);
    //const utcNow = Date.now();
    const utcNow = 1433859075852;

    // find difference and convert milliseconds to minutes
    const differenceMs = (utcArrival - utcNow);


    const min = Math.floor(differenceMs/1000/60);
    const sec = Math.floor((differenceMs/1000) % 60);

    //return min + ':' + sec;
    return min + 'm ' + sec + 's';
}



if (require.main === module) {

    //const tta = getTimeToArrival(dateString);
    //console.log(tta);

    const a = [];
    console.log(a);
    a.push(5);
    console.log(a);

    console.log(a);

}