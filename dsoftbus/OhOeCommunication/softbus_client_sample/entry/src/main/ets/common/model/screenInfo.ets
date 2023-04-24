import display from '@ohos.display'
import hilog from '@ohos.hilog';
import { INTERVAL_NUM, INTERVAL_DISTANCE, IMAGE_OF_ROW } from '../../common/model/constant'

export default class ScreenInfo {
  //获取屏幕信息，获取屏幕尺寸
  getScreenInfo(callback) {
    hilog.debug(0, "SoftBusClient", `[INFO] Start to obtain all the display`)
    let displayClass = null
    display.getAllDisplay((err, data) => {
      if (err.code) {
        hilog.error(0, "SoftBusClient", `[ERROR] Failed to obtain all the display objects: ${JSON.stringify(err)}`)
        return;
      }

      hilog.debug(0, "SoftBusClient", `[INFO] data = ${JSON.stringify(data)}`)
      displayClass = data[0];
      hilog.debug(0, "SoftBusClient", `[INFO] displayClass = ${JSON.stringify(displayClass)}`)
      let displayWidth = displayClass.width
      hilog.info(0, "SoftBusClient", `[INFO] displayWidth = ${displayWidth}`)
      let displayHeight = displayClass.height
      hilog.info(0, "SoftBusClient", `[INFO] displayHeight = ${displayHeight}`)
      let screenWidth = displayWidth > displayHeight ? displayHeight : displayWidth
      callback((screenWidth - INTERVAL_DISTANCE * INTERVAL_NUM) / IMAGE_OF_ROW)
    })
  }
}

