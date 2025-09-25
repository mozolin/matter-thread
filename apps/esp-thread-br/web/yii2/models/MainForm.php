<?
namespace app\models;

use yii\base\Model;

class MainForm extends Model
{
  
  public $success = false;
  public $search_word = '';
  public $empty = false;

  public function rules()
  {
    return [
      [['search_word'], 'string', 'min' => 2, 'tooShort' => 'Слишком короткое значение'],
      [['search_word'], 'string', 'max' => 50, 'tooLong' => 'Слишком длинное значение'],
      [['search_word'], 'required', 'message' => 'Обязательное поле'],
    ];
  }

}
