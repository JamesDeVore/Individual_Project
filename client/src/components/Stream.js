import React, { Component } from 'react'
import PropTypes from 'prop-types'
import { connect } from 'react-redux'
import Data from './StreamData'
import * as actions from "../actions";


class Stream extends Component {
  constructor(props){
    super(props)
  }

  _handleCharacteristicValueChanged = (event) => {
    
    let dataString = "";
    //this function reads the incoming bytes
    // console.log(event)
    let buffer = event.target.value.buffer;
    var dataView = new DataView(buffer);
    let data = "";
    for (let i = 0; i < event.target.value.byteLength; i++) {
      let byte = dataView.getInt8(i)
      let character = String.fromCharCode(byte);
      data += character;
    }
    this.props.streamData(data);
  }
  connectBTDevice = async () => {
    let options = {
      acceptAllDevices:true,
      optionalServices: ["6e400001-b5a3-f393-e0a9-e50e24dcca9e"]
    };
    console.log('Requesting Bluetooth Device...');
    let device = await navigator.bluetooth.requestDevice(options);
    let server = await device.gatt.connect();
    let service = await server.getPrimaryService('6e400001-b5a3-f393-e0a9-e50e24dcca9e')
    let characteristic = await service.getCharacteristic('6e400003-b5a3-f393-e0a9-e50e24dcca9e')
    await characteristic.startNotifications()
    await characteristic.addEventListener('characteristicvaluechanged',this._handleCharacteristicValueChanged);
    console.log('Device connected, listening for events...');
  }

render() {
  // console.log(this.props)
  return (
    <div>
      <button onClick={() => this.connectBTDevice()}>Connect</button>
      <Data />
    </div>
  )
}
}


const mapStateToProps = ({ stream }) => {
  return { stream };
};

export default 
  connect(
    null,
    actions
  )(Stream)
