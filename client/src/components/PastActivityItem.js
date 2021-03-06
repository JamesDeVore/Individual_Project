import React, { Component } from 'react'
import MapItem from "./map";
import moment from 'moment';
import C3Chart from "react-c3js";
import { C3Speed, C3Past, C3Altitude } from '../utils/C3';
import CalorieCountup from "./CalorieCountup";


/*=====================================================
individual past activity display, to be mapped over
=====================================================*/
export default class PastActivity extends Component {
  constructor(props){
    super(props)
    this.state = {
      totalCal : this.props.allData.calories
    }
  }


  render() {
    let { allData: { coordinates, speed, calories, averageSpeedMPH, totalDistanceM, date, altitude } } = this.props

        

    console.log(this.props)
    speed.unshift('Speed')
    altitude.unshift('Altitude')
    const data = {
      columns: [speed],
      colors: {
        Speed: '#FF6F6E'
      }
    }
    let tooltip = {
      show: false
    }
    const altData = { columns: [altitude], colors: { Altitude: "#4252E5" } };

    
    return <div className="recentActivity m-4 ">
        <div>
          <h1 className="font-thin items-center mb-10 mr-8">
            {moment(date).format("MMMM Do, h:mm a")}
          </h1>
        </div>
        <div className="flex flex-row justify-between">
          <div className="past-map ml-10 mr-10 w-1/6">
            <MapItem coordinates={coordinates} />
          </div>
          <div className=" flex-col mr-10 justify-between">
            <h2 className="font-semibold">Total Distance:</h2>
            <h2 className="font-thin text-2xl">
              {Math.round(totalDistanceM * 100) / 100} miles
            </h2>
            <hr />
            <h2 className="font-semibold"> Average Speed:</h2>
            <h2 className="font-thin text-2xl">
              {Math.round(averageSpeedMPH * 100) / 100} mph
            </h2>
          <CalorieCountup calories={calories} />
          </div>
          <div className="shadow pt-4 flex w-1/3 flex-col flex-wrap justify-center rounded border">
          <div className="flex flex-row justify-center">
            <div className="ml-2 mr-2">
              <C3Chart className="no-tick" data={data} size={C3Past.size} axis={C3Speed.axis} point={{ r: 0 }} tooltip={tooltip} />
            </div>
            <div className="ml-2 mr-2">
              <C3Chart className="no-tick" data={altData} size={C3Past.size} axis={C3Altitude.axis} point={{ r: 0 }} tooltip={tooltip} />
            </div>
           
          </div>
          <div className=" content-center flex flex-col justify-center ">
          
            </div>
          </div>
        </div>
        <hr />
      </div>;
  }
}


