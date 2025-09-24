<?
namespace app\controllers;

use yii\web\Controller;
use yii\web\Response;

class PostController extends Controller
{
  
  public function actionEsptoolFlashId()
  {
    $comPort = \Yii::$app->request->post('com-port');
    
    $req = [];
    if(\Yii::$app->request->isAjax) {
      \Yii::$app->response->format = Response::FORMAT_JSON;
      $req = [
      	'comPort' => $comPort,
  	  ];
    }

    //return $this->render('esptool-flash-id');
    return $this->renderPartial('esptool-flash-id', $req);
  }

}
