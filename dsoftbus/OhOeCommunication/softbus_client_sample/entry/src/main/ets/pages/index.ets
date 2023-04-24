import router from '@ohos.router';
import prompt from '@ohos.prompt';
import hilog from '@ohos.hilog';
import softbus_client_run from 'libsoftbus_client_run.so'
import { AI_ANALYSIS_RESULT, SITUATION_PERCEPTION_REALTIME } from '../common/model/constant'

var thisOld

@Entry
@Component
struct Index {
  @State sessionServerInitState: string = "null"
  @State sessionConnectState: string = "null"
  @State sendDataState: string = ''
  @State pictureFile: string = ''
  @State hasNewAIImage: boolean = false
  @State hasNewSAImage: boolean = false
  @State isLoading: boolean = false
  @State deviceInfoArray: Array<any> = []
  @State deviceIndex: number = undefined

  callbackPictureSave(fileName) {
    hilog.info(0, "SoftBusClient", "[INFO] Image file received: %s", fileName)
    if (fileName.search("ai_analysis") != -1) {
      thisOld.hasNewAIImage = true
    } else {
      if (fileName.search("situation_awareness") != -1) {
        thisOld.hasNewSAImage = true
      }
    }
  }

  build() {
    Stack() {
      Column() {
        Row() {
          if (thisOld.hasNewAIImage) {
            Text($r('app.string.hasNewImage'))
              .fontSize(16)
              .fontColor($r('app.color.white'))
              .onClick(() => {
                thisOld.hasNewAIImage = false
                router.push({
                  url: 'pages/AIImages',
                  params: { "from": AI_ANALYSIS_RESULT }
                })
              })
          }
          if (thisOld.hasNewSAImage) {
            Text($r('app.string.hasNewSAImage'))
              .fontSize(16)
              .fontColor($r('app.color.white'))
              .onClick(() => {
                thisOld.hasNewSAImage = false
                router.push({
                  url: 'pages/AIImages',
                  params: { "from": SITUATION_PERCEPTION_REALTIME }
                })
              })
          }
        }
        .justifyContent(FlexAlign.Center)
        .width('100%')
        .height('8%')
        .padding(10)
        .backgroundColor($r('app.color.transparent_grey'))

        Row() {
          Button({ type: ButtonType.Circle, stateEffect: true }) {
            if (this.isLoading) {
              LoadingProgress()
                .color(Color.White)
                .width(70)
                .height(70)
            } else {
              Image($r('app.media.refresh'))
                .width(48)
                .height(48)
                .objectFit(ImageFit.Fill)
            }
          }
          .width(64).height(64)
          .backgroundColor($r('app.color.transparent'))
          .onClick(() => {
            this.isLoading = true
            this.deviceInfoArray.length = 0
            this.deviceIndex = undefined
            this.sessionConnectState = softbus_client_run.sessionConnect()
            this.isLoading = false
            this.parseDeviceInfo(this.sessionConnectState)
          })
        }
        .width('90%')
        .height('8%')
        .justifyContent(FlexAlign.Start)

        Column() {
          Text('终端：' + this.deviceInfoArray.length.toString())
            .fontColor($r('app.color.white'))
            .fontSize(16)
          List() {
            ForEach(this.deviceInfoArray, (items) => {
              ListItem() {
                Row() {
                  Column() {
                    Image($r('app.media.phone'))
                      .width(42)
                      .height(42)
                      .objectFit(ImageFit.Cover)
                      .onClick(() => {
                        this.deviceIndex = this.deviceInfoArray.indexOf(items)
                      })
                    Text(items[0]).maxLines(2).fontColor($r('app.color.white'))
                      .fontSize(16)
                      .textAlign(TextAlign.Center)
                      .margin({ left: 12 })
                  }

                  if (this.deviceIndex == this.deviceInfoArray.indexOf(items)) {
                    Text(items[1])
                      .fontColor($r('app.color.white'))
                      .fontSize(20)
                      .textAlign(TextAlign.Start)
                      .margin({ left: 12 })
                  }
                }
                .width('100%')
                .margin(8)
              }
            })
          }.padding(2).height('82%')
        }
        .alignItems(HorizontalAlign.Start)
        .margin({ top: 20 })
        .width('72%')
        .height('48%')

        Row() {
          Column() {
            Text('无人机控制：')
              .fontSize(24)
              .margin(5)
              .fontColor($r('app.color.white'))
            Button($r('app.string.UAV_take_off'), { type: ButtonType.Normal })
              .borderRadius(8)
              .fontSize(24)
              .height('18%')
              .width('60%')
              .backgroundColor($r('app.color.button_color'))
              .margin(5)
              .onClick(() => {
                let sendResult = softbus_client_run.sendData("UAV", "takeoff", 7)
                if (sendResult == 0) {
                  this.sendDataState = "Send Successful"
                } else {
                  this.sendDataState = "Send Failed"
                }
                prompt.showToast({
                  message: this.sendDataState,
                  bottom: '20px'
                })
              })
            Button($r('app.string.UAV_hover'), { type: ButtonType.Normal })
              .borderRadius(8)
              .fontSize(24)
              .height('18%')
              .width('60%')
              .backgroundColor($r('app.color.button_color'))
              .margin(5)
              .onClick(() => {
                let sendResult = softbus_client_run.sendData("UAV", "hover", 5)
                if (sendResult == 0) {
                  this.sendDataState = "Send Successful"
                } else {
                  this.sendDataState = "Send Failed"
                }
                prompt.showToast({
                  message: this.sendDataState,
                  bottom: '20px'
                })
              })
            Button($r('app.string.UAV_land'), { type: ButtonType.Normal })
              .borderRadius(8)
              .fontSize(24)
              .height('18%')
              .width('60%')
              .backgroundColor($r('app.color.button_color'))
              .margin(5)
              .onClick(() => {
                let sendResult = softbus_client_run.sendData("UAV", "land", 4)
                if (sendResult == 0) {
                  this.sendDataState = "Send Successful"
                } else {
                  this.sendDataState = "Send Failed"
                }
                prompt.showToast({
                  message: this.sendDataState,
                  bottom: '20px'
                })
              })
          }
          .justifyContent(FlexAlign.SpaceEvenly)
          .width('50%')
          .height('100%')

          Column() {
            Text('机器狗控制:')
              .fontSize(24)
              .margin(5)
              .fontColor($r('app.color.white'))
            Button($r('app.string.Robot_dog_up'), { type: ButtonType.Normal })
              .borderRadius(8)
              .fontSize(24)
              .height('18%')
              .width('30%')
              .backgroundColor($r('app.color.button_color'))
              .margin(5)
              .onClick(() => {
                let sendResult = softbus_client_run.sendData("Robot_dog", "forward", 7)
                if (sendResult == 0) {
                  this.sendDataState = "Send Successful"
                } else {
                  this.sendDataState = "Send Failed"
                }
                prompt.showToast({
                  message: this.sendDataState,
                  bottom: '20px'
                })
              })
            Row() {
              Button($r('app.string.Robot_dog_left'), { type: ButtonType.Normal })
                .borderRadius(8)
                .fontSize(24)
                .height('60%')
                .width('30%')
                .backgroundColor($r('app.color.button_color'))
                .margin(5)
                .onClick(() => {
                  let sendResult = softbus_client_run.sendData("Robot_dog", "left", 4)
                  if (sendResult == 0) {
                    this.sendDataState = "Send Successful"
                  } else {
                    this.sendDataState = "Send Failed"
                  }
                  prompt.showToast({
                    message: this.sendDataState,
                    bottom: '20px'
                  })
                })
              Button($r('app.string.Robot_dog_right'), { type: ButtonType.Normal })
                .borderRadius(8)
                .fontSize(24)
                .height('60%')
                .width('30%')
                .backgroundColor($r('app.color.button_color'))
                .margin(5)
                .onClick(() => {
                  let sendResult = softbus_client_run.sendData("Robot_dog", "right", 5)
                  if (sendResult == 0) {
                    this.sendDataState = "Send Successful"
                  } else {
                    this.sendDataState = "Send Failed"
                  }
                  prompt.showToast({
                    message: this.sendDataState,
                    bottom: '20px'
                  })
                })
            }
            .justifyContent(FlexAlign.SpaceEvenly)
            .width('100%')
            .height('30%')
            Button($r('app.string.Robot_dog_down'), { type: ButtonType.Normal })
              .borderRadius(8)
              .fontSize(24)
              .height('18%')
              .width('30%')
              .backgroundColor($r('app.color.button_color'))
              .margin(5)
              .onClick(() => {
                let sendResult = softbus_client_run.sendData("Robot_dog", "backward", 8)
                if (sendResult == 0) {
                  this.sendDataState = "Send Successful"
                } else {
                  this.sendDataState = "Send Failed"
                }
                prompt.showToast({
                  message: this.sendDataState,
                  bottom: '20px'
                })
              })
          }
          .justifyContent(FlexAlign.SpaceEvenly)
          .width('50%')
          .height('100%')
        }
        .height('34%')
        .width('100%')
        .justifyContent(FlexAlign.SpaceEvenly)
        .padding({ bottom: 8 })
        .backgroundColor('#07309E')
      }
      .width('100%')
      .height('100%')
      .backgroundImage($r('app.media.background'))
      .backgroundImageSize({ width: '100%', height: '75%' })
    }
  }

  aboutToAppear() {
    //进入页面初始化
    thisOld = this
    this.sessionServerInitState = softbus_client_run.sessionServerInit(this.callbackPictureSave)
    prompt.showToast({
      message: this.sessionServerInitState,
      bottom: '20px'
    })
  }

  aboutToDisappear() {
    //关闭
    softbus_client_run.sessionDisconnect()
  }

  parseDeviceInfo(deviceInfo: string) {
    let deviceInfoArrayTemp = deviceInfo.split(';')
    deviceInfoArrayTemp.pop()
    deviceInfoArrayTemp.forEach(element => {
      let startIndex = element.indexOf(': ')
      let endIndex = element.indexOf('\nipAddress')
      let deviceName = element.substring(startIndex + 1, endIndex)
      this.deviceInfoArray.push([deviceName, element])
    });
  }
}