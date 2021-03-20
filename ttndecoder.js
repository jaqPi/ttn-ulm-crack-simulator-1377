function Decoder(bytes, port) {
    // Decode an uplink message from a buffer
    // (array) of bytes to an object of fields.
    var decoded = {};
  
    switch(bytes[0]) {
      case 0:
        decoded.temp1 = ((bytes[1] << 8) + bytes[2]) / 100;
        decoded.pressure1 = ((bytes[3] << 8) + bytes[4]);
        decoded.humidity1 = ((bytes[5] << 8) + bytes[6]) / 100;
        
        decoded.temp2 = ((bytes[7] << 8) + bytes[8]) / 100;
        decoded.pressure2 = ((bytes[9] << 8) + bytes[10]);
        decoded.humidity2 = ((bytes[11] << 8) + bytes[12]) / 100;
        break;
        
      case 1:
      case 2:
      case 3:
      case 4:
        decoded["calibrationModeAuto" + bytes[0]] = (bytes[1] & 1) === 1;
        decoded["measureModeOneByOne" + bytes[0]] = (bytes[1] & 2) === 2;
        
        decoded["meanDistance" + bytes[0]] = ((bytes[2] << 8) + bytes[3]) / 100;
        decoded["standardDeviationDistance" + bytes[0]] = ((bytes[4] << 8) + bytes[5]) / 100;
        decoded["medianDistance" + bytes[0]] = ((bytes[6] << 8) + bytes[7]) / 100;
        decoded["successfulMeasurementsDistance" + bytes[0]] = parseInt(bytes[8],10);
        
        decoded["meanAmbientLight" + bytes[0]] = ((bytes[9] << 8) + bytes[10]) / 100;
        decoded["standardDeviationAmbientLight" + bytes[0]] = ((bytes[11] << 8) + bytes[12]) / 100;
        decoded["medianAmbientLight" + bytes[0]] = ((bytes[13] << 8) + bytes[14]) / 100;
        decoded["successfulMeasurementsAmbientLight" + bytes[0]] = parseInt(bytes[15],10);
        break;
    // ERROR
      case 5:
        decoded.sensor = bytes[1];
        decoded.errorType = bytes[2];
        if(errorType != 2) {
            decoded.errorCode = bytes[3];
        }
      break;
    }
  
    return decoded;
  }