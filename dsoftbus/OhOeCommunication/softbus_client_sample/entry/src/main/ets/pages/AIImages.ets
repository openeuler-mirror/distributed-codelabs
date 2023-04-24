import hilog from '@ohos.hilog';
import router from '@ohos.router';
import ImageData from '../common/model/imagesData'
import ScreenInfo from '../common/model/screenInfo'
import { INTERVAL_DISTANCE, AI_ANALYSIS_RESULT } from '../common/model/constant'


@Entry
@Component
export default struct AIImages {
  @State imageBgColor: ResourceStr = $r('app.color.transparent')
  @State imageIsClick: boolean = false
  @State imageIndex: number = 0
  @State photoData: Array<string> = []
  private swiperController: SwiperController = new SwiperController()
  private imageData: ImageData = new ImageData()
  private screenInfo: ScreenInfo = new ScreenInfo()

  build() {
    Stack() {
      if (this.imageIsClick) {
        Column() {
          Swiper(this.swiperController) {
            ForEach(this.photoData, (item: string) => {
              Image(item).width('100%').height('100%')
                .objectFit(ImageFit.Contain).padding(8)
                .onClick(() => {
                  this.imageIsClick = false
                })
            }, item => item)
          }
          .cachedCount(1)
          .indicator(false) //是否开启导航点指示器，默认开启true
          .index(this.imageIndex)
          .loop(false) // 默认开启循环播放
          .duration(100)
          .vertical(false) // 默认横向切换
          .itemSpace(0)
          .curve(Curve.FastOutSlowIn) // 动画曲线
          .onChange((index: number) => {
            console.info(index.toString())
            this.imageIndex = index
            console.info(`ypt---Swiper index = ${index}`)
          })
        }
        .width('100%')
        .height('100%')
        .zIndex(6)
        .backgroundColor(Color.White)
      }
      Grid() {
        ForEach(this.photoData, item => {
          GridItem() {
            Image(item)
              .objectFit(ImageFit.Cover)
              .width('100%')
              .height('12%')
              .onClick(() => {
                this.imageIsClick = true
                this.imageIndex = this.photoData.indexOf(item)
                console.info(`ypt---index = ${this.photoData.indexOf(item)}`)
              })
          }
        })
      }
      .width('100%')
      .height('90%')
      .margin({ top: '10%' })
      .columnsTemplate('1fr 1fr 1fr 1fr')
      .columnsGap(5)
      .rowsGap(5)
      .onScrollIndex((first: number) => {
        console.info(first.toString())
      })
      .width('100%')
      .height('90%')

      Row() {
        Image($r('app.media.back'))
          .width(32)
          .height(32)
          .borderRadius(32)
          .objectFit(ImageFit.Cover)
          .margin({ left: 8 })
          .backgroundColor(this.imageBgColor)
          .onClick(() => {
            this.imageBgColor = $r('app.color.transparent_grey')
            router.back()
          })
      }
      .width('100%')
      .height('8%')
      .zIndex(3)
      .offset({ y: '-45%' })
      .justifyContent(FlexAlign.Start)
    }
  }

  aboutToAppear() {
    let self
    self = this
    //判断从哪来的
    let router_params = router.getParams()
    if (router_params["from"] == AI_ANALYSIS_RESULT) {
      //首次进入获取图片
      self.imageData.getAIImageData(callback => {
        self.photoData = callback
        self.photoData.reserve()
      })
    } else {
      self.imageData.getSituationImageData(callback => {
        self.photoData = callback
        self.photoData.reserve()
      })
    }
  }

  aboutToDisappear() {
    router.clear()
  }
}