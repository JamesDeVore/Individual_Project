exports.calculateTotalDistance = coordsArray => {
  let totalDistanceInKm = 0;
  for (let i = 0; i < coordsArray.length-1; i++) {
    let lat1 = parseFloat(coordsArray[i][1]);
    let lon1 = parseFloat(coordsArray[i][0]);
    let lat2 = parseFloat(coordsArray[i + 1][1]);
    let lon2 = parseFloat(coordsArray[i + 1][0]);
    var R = 6371; // Radius of the earth in km
    var dLat = _deg2rad(lat2 - lat1); // deg2rad below
    var dLon = _deg2rad(lon2 - lon1);
    var a =
      Math.sin(dLat / 2) * Math.sin(dLat / 2) +
      Math.cos(_deg2rad(lat1)) *
        Math.cos(_deg2rad(lat2)) *
        Math.sin(dLon / 2) *
        Math.sin(dLon / 2);
    var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    var d = R * c; // Distance in km
   
    totalDistanceInKm = totalDistanceInKm + d;
  }
  let totalDistanceInMiles = (totalDistanceInKm/1.609) //convert to miles
  return totalDistanceInMiles;
};

const _deg2rad = deg => {
  return deg * (Math.PI / 180);
};

exports.calculateAvgSpeed = (speedArray) => {
  let sum = speedArray.reduce((speed1,speed2) => speed1+speed2);
  let avg = sum/speedArray.length;
  return avg
}

exports.remove$FromCoords = element => {
  let newDataArray = [];
  newDataArray.push(parseFloat(element[0]['$numberDecimal']));
  newDataArray.push(parseFloat(element[1]['$numberDecimal']));
  return newDataArray
}

//functions to be passed into the array.reduce 
exports.sumTotalDistance = (total, currentVal) => {
  return total + parseFloat(currentVal.totalDistanceM)
}

exports.findFastestPace = (total, currentVal) => {
  if (currentVal.pace > total.pace) {
    return currentVal
  } else {
    return total
  }
}

exports.sumTotalCalories = (total, currentVal) => {
  return total + parseInt(currentVal.calories)
}

exports.findAverageSpeed = (total, currentVal) => {
  return total + parseFloat(currentVal.averageSpeedMPH)
}
