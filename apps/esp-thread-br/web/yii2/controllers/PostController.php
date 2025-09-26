<?
namespace app\controllers;

use yii\web\Controller;
use yii\web\Response;

class PostController extends Controller
{
  
  public function actionEsptoolFlashId()
  {
    $comPort = \Yii::$app->request->post('com-port');
    
    $params = [];
    if(\Yii::$app->request->isAjax) {
      \Yii::$app->response->format = Response::FORMAT_JSON;
      $params = [
      	'comPort' => $comPort,
  	  ];
    }

    //return $this->render('esptool-flash-id');
    return $this->renderPartial('esptool-flash-id', $params);
  }

  public function actionFormSubmit()
  {
    $data = \Yii::$app->request->post();

    $params = [];
    //if(\Yii::$app->request->isAjax) {
      \Yii::$app->response->format = Response::FORMAT_JSON;
      $params = [
      	'data' => $data,
  	  ];
    //}

    return $this->renderPartial('form-submit', $params);
  }

  public function actionPartitionsSubmit()
  {
    $data = \Yii::$app->request->post();

    $params = [];
    //if(\Yii::$app->request->isAjax) {
      \Yii::$app->response->format = Response::FORMAT_JSON;
      $params = [
      	'data' => $data,
  	  ];
    //}

    return $this->renderPartial('partitions-submit', $params);
  }

}
